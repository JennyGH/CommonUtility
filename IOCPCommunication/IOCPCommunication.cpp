#include "stdafx.h"
#include "IOCPCommunication.h"
#include "OverlappedWrapper.h"
#include "AsyncConnectionWrapper.h"
#include "ThreadHelper.h"
#include "AsyncConnectionWrapperContainer.h"

#define LOCK \
((CriticalSection*)m_hCriticalSection)->Lock();\
common::raii::scope_member_function_noparam<CriticalSection, void> scope_lock((CriticalSection*)m_hCriticalSection, &CriticalSection::UnLock)

static GUID									g_guidAcceptEx = WSAID_ACCEPTEX;
static GUID									g_guidDisconnectEx = WSAID_DISCONNECTEX;
static GUID									g_guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
static LPFN_ACCEPTEX						g_lpfnAcceptEx = NULL;
static LPFN_DISCONNECTEX					g_lpfnDisconnectEx = NULL;
static LPFN_GETACCEPTEXSOCKADDRS			g_lpfnGetAcceptExSockaddrs = NULL;
static UINT32								GetCountOfProcessors();
static int32_t								GetFunctionByGuid(UINT_PTR socket, GUID& guid, LPVOID lpFn, DWORD nSizeOfFn);
#define GetFunction(socket, guid, lpFn)		GetFunctionByGuid(socket, guid, lpFn, sizeof(lpFn))

IOCPCommunication::IOCPCommunication(const IOCPSettings& settings, IConnectionFactory *pConnectionFactory) :
	m_nAF(settings.af),
	m_nProtocol(settings.protocol),
	m_nSendBufferSize(settings.nSendBufferSize),
	m_nRecvBufferSize(settings.nRecvBufferSize <= 0 ? DEFAULT_RECV_BUFFER_SIZE : settings.nRecvBufferSize),
	m_nSocket(INVALID_SOCKET),
	m_pAddress(NULL),
	m_hCompletionPort(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)),
	m_hCriticalSection(new CriticalSection()),
	m_hConnectionContainer(new AsyncConnectionWrapperContainer()),
	m_nCountOfThreads(settings.nCountOfThreads > 0 ? settings.nCountOfThreads : GetCountOfProcessors()),
	m_pFactory(pConnectionFactory),
	m_isAcceptWithData(settings.isAcceptWithData)
{
	WSAData wsaData;
	if (SOCKET_ERROR == WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		OnError(NULL, WSAGetLastError());
		return;
	}
	if (INVALID_SOCKET == (m_nSocket = WSASocket(m_nAF, SOCK_STREAM, m_nProtocol, NULL, 0, WSA_FLAG_OVERLAPPED)))
	{
		OnError(NULL, WSAGetLastError());
		return;
	}
	if (SOCKET_ERROR == GetFunction(m_nSocket, g_guidAcceptEx, &g_lpfnAcceptEx))
	{
		OnError(NULL, WSAGetLastError());
		return;
	}
	if (SOCKET_ERROR == GetFunction(m_nSocket, g_guidDisconnectEx, &g_lpfnDisconnectEx))
	{
		OnError(NULL, WSAGetLastError());
		return;
	}
	if (SOCKET_ERROR == GetFunction(m_nSocket, g_guidGetAcceptExSockaddrs, &g_lpfnGetAcceptExSockaddrs))
	{
		OnError(NULL, WSAGetLastError());
		return;
	}
}

IOCPCommunication::~IOCPCommunication()
{
	Shutdown();

	::CloseHandle(m_hCompletionPort);

	if (INVALID_SOCKET != m_nSocket)
	{
		closesocket(m_nSocket);
		m_nSocket = INVALID_SOCKET;
	}
	WSACleanup();

	common::raii::release_object((IConnectionFactory**)&m_pFactory);
	common::raii::release_object((CriticalSection**)&m_hCriticalSection);
	common::raii::release_object((AsyncConnectionWrapperContainer**)&m_hConnectionContainer);
}

static int32_t GetFunctionByGuid(UINT_PTR socket, GUID& guid, LPVOID lpFn, DWORD nSizeOfFn)
{
	DWORD dwBytes = 0;
	return WSAIoctl(
		socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guid, sizeof(guid),
		lpFn, nSizeOfFn,
		&dwBytes, NULL, NULL);
}
static UINT32 GetCountOfProcessors()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

unsigned WINAPI IOCPCommunication::_WorkerThread(void* hHandle)
{
	if (hHandle == NULL)
	{
		return ERROR_BAD_ARGUMENTS;
	}

	IOCPCommunication& self = *(IOCPCommunication*)hHandle;

	AsyncConnectionWrapperContainer& container = *(AsyncConnectionWrapperContainer*)self.m_hConnectionContainer;

	while (true)
	{
		DWORD nBytesOfTransferred = 0;
		OVERLAPPED* pOverlapped = NULL;
		AsyncConnectionWrapper* pConnection = NULL; //hCompletionKey for CreateIoCompletionPort.
		BOOL bSuccess = ::GetQueuedCompletionStatus(
			self.m_hCompletionPort,
			&nBytesOfTransferred,
			(PULONG_PTR)&pConnection,
			(LPOVERLAPPED*)&pOverlapped,
			WSA_INFINITE);

		if (nBytesOfTransferred == -1)
		{
			//::SetEvent(self.m_hShutdownEvent);
			return 0; //exit.
		}

		OverlappedWrapper* buffer = (OverlappedWrapper*)pOverlapped;
		if (pOverlapped == NULL)
		{
			continue;
		}
		//这次作用域结束后移除对应buffer
		common::raii::scope_function<void, const OverlappedWrapper&> scope_remove(OverlappedWrapper::RemoveBuffer, *buffer);

		if (!bSuccess)
		{
			self.OnError(
				buffer->owner->Connection(),
				buffer->GetErrorCode()
			);
			continue;
		}
		//Client disconnected.
		if (0 == nBytesOfTransferred &&
			buffer != NULL &&
			buffer->operation != AsyncOperation::Accept)
		{
			if (buffer->operation == AsyncOperation::Disconnect)
			{
				self.OnRemoved(buffer->owner->Connection());
			}
			else
			{
				self.OnClientLost(buffer->owner->Connection());
			}
			if (self.m_hConnectionContainer != NULL)
			{
				//回收 AsyncConnectionWrapper，在下一次复用
				container.RecycleConnection(buffer->owner);
			}
			AsyncConnectionWrapper::Remove(buffer->owner->Connection());
			continue;
		}

		switch (buffer->operation)
		{
		case AsyncOperation::Accept:
		{
			//投递accept信号，使得下一轮循环可以accept到新的连接
			self.PostAccept();

			int nLocalAddrLength = 0;
			int nRemoteAddrLength = 0;
			SOCKADDR* pLocalAddr = NULL;
			SOCKADDR* pRemoteAddr = NULL;

			//获取IP地址
			g_lpfnGetAcceptExSockaddrs(
				buffer->wsaBuffer.buf,
				(self.m_isAcceptWithData ? buffer->wsaBuffer.len - ((buffer->owner->Connection()->GetSizeOfAddress() + 16) * 2) : 0),
				buffer->owner->Connection()->GetSizeOfAddress() + 16,
				buffer->owner->Connection()->GetSizeOfAddress() + 16,
				&pLocalAddr, &nLocalAddrLength,
				&pRemoteAddr, &nRemoteAddrLength
			);

			//设置连接的IP地址
			buffer->owner->Connection()->SetAddress(pRemoteAddr, nRemoteAddrLength);

			//Set socket options.
			{
				LINGER lingerStruct;
				lingerStruct.l_onoff = 1;
				lingerStruct.l_linger = 0;
				::setsockopt(buffer->owner->Socket(), SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));
				::setsockopt(buffer->owner->Socket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&(self.m_nSocket), sizeof(self.m_nSocket));
			}
			//AsyncConnectionWrapper* ptr = &(*buffer->owner);

			//将这个连接的socket绑定到完成端口上
			::CreateIoCompletionPort((HANDLE)buffer->owner->Socket(), self.m_hCompletionPort, /*(ULONG_PTR)&(ptr)*/ NULL, 0);

			//投递recv信号，如果收到数据，将在下一次循环时进入 case AsyncOperation::Recv
			self.PostRecv(&(*buffer->owner));

			//accept回调
			self.OnAccepted(
				buffer->owner->Connection(),
				(const BYTE*)buffer->wsaBuffer.buf,
				nBytesOfTransferred
			);

			break;
		}
		case AsyncOperation::Recv:
		{
			//继续投递recv信号，接收下一轮数据
			self.PostRecv(&(*buffer->owner));

			self.OnRecved(
				buffer->owner->Connection(),
				(const BYTE*)buffer->wsaBuffer.buf,
				nBytesOfTransferred
			);
			break;
		}
		case AsyncOperation::Send:
		{
			self.OnSent(
				buffer->owner->Connection(),
				(const BYTE*)buffer->wsaBuffer.buf,
				nBytesOfTransferred
			);
			break;
		}
		case AsyncOperation::Disconnect:
		{
			break;
		}
		default:
			break;
		}
	}

	return 0;
}

void IOCPCommunication::PostAccept()
{
	AsyncConnectionWrapperContainer& container = *(AsyncConnectionWrapperContainer*)m_hConnectionContainer;
	DWORD dwBytes = 0;
	IConnectionBase* conn = NULL;
	AsyncConnectionWrapperPtr asyncConn = NULL;
	//如果连接对象容器不为空
	if (m_hConnectionContainer != NULL && container.GetCountOfConnections() > 0)
	{
		//从里面拿一个连接对象
		asyncConn = container.GetConnection();
		conn = asyncConn->Connection();
	}
	else
	{
		conn = m_pFactory->NewConnection();
		asyncConn.reset(new AsyncConnectionWrapper(conn, m_nAF, m_nProtocol));
	}

	OverlappedWrapper& buffer = OverlappedWrapper::NewBuffer(asyncConn, AsyncOperation::Accept, m_nRecvBufferSize);
	BOOL bSucess = g_lpfnAcceptEx(
		m_nSocket,
		asyncConn->Socket(),
		buffer.wsaBuffer.buf,
		(m_isAcceptWithData ? buffer.wsaBuffer.len - ((conn->GetSizeOfAddress() + 16) * 2) : 0),
		conn->GetSizeOfAddress() + 16,
		conn->GetSizeOfAddress() + 16,
		&dwBytes,
		&buffer.overlapped
	);
	if (!bSucess)
	{
		DWORD err = WSAGetLastError();
		if (WSA_IO_PENDING != err && 0 != err)
		{
			OnError(conn, err);
			return;
		}
	}
	return;
}
void IOCPCommunication::PostRecv(HANDLE hHandle)
{
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;
	if (hHandle == NULL)
	{
		OnError(NULL, ERROR_INVALID_HANDLE);
		return;
	}
	AsyncConnectionWrapper* connection = (AsyncConnectionWrapper*)hHandle;
	OverlappedWrapper& buffer = OverlappedWrapper::NewBuffer(AsyncConnectionWrapperPtr(connection), AsyncOperation::Recv, m_nRecvBufferSize);
	if (SOCKET_ERROR == WSARecv(connection->Socket(), &(buffer.wsaBuffer), 1, &dwBytes, &dwFlags, &buffer.overlapped, NULL))
	{
		UINT64 errcode = buffer.GetErrorCode();
		if (errcode != WSA_IO_PENDING && errcode != WSA_IO_INCOMPLETE && errcode != 0)
		{
			OnError(connection->Connection(), errcode);
			return;
		}
	}
	return;
}

void IOCPCommunication::Startup(const void * addr, UINT32 sizeOfAddress)
{
	if (addr == NULL)
	{
		OnError(NULL, ERROR_BAD_ARGUMENTS);
		return;
	}
	if (m_pFactory == NULL)
	{
		OnError(NULL, ERROR_NO_CONNECTION_FACTORY);
		return;
	}

	sockaddr* address = (sockaddr*)addr;
	address->sa_family = m_nAF;
	if (SOCKET_ERROR == ::bind(m_nSocket, address, sizeOfAddress))
	{
		OnError(NULL, WSAGetLastError());
		return;
	}
	//创建完成端口
	CreateIoCompletionPort((HANDLE)m_nSocket, m_hCompletionPort, 0, 0);
	if (SOCKET_ERROR == ::listen(m_nSocket, SOMAXCONN))
	{
		OnError(NULL, WSAGetLastError());
		return;
	}
	//创建若干个工作线程
	{
		thread_t thread = { _WorkerThread, this };
		ThreadHelper::Create(thread, m_nCountOfThreads);
	}
	//投递Accept信号
	return PostAccept();
}
void IOCPCommunication::Send(const IConnectionBase* pConnection, const BYTE data[], UINT32 size)
{
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;
	UINT32 offset = 0;
	if (pConnection == NULL)
	{
		OnError(pConnection, ERROR_INVALID_CONNECTION);
		return;
	}
	AsyncConnectionWrapperPtr connection = AsyncConnectionWrapper::Find(pConnection);
	if (connection == NULL)
	{
		OnError(pConnection, ERROR_INVALID_CONNECTION);
		return;
	}

	if (m_nSendBufferSize == UINT32(-1) || m_nSendBufferSize == 0)
	{
		OverlappedWrapper& buffer = OverlappedWrapper::NewBuffer(connection, AsyncOperation::Send, size);
		memcpy_s(buffer.wsaBuffer.buf, buffer.wsaBuffer.len, data, size);
		if (SOCKET_ERROR == WSASend(connection->Socket(), &(buffer.wsaBuffer), 1, &dwBytes, dwFlags, &buffer.overlapped, NULL))
		{
			UINT64 errcode = buffer.GetErrorCode();
			OnError(connection->Connection(), errcode);
			buffer.owner = NULL;
			return;
		}
	}
	else
	{
		//Split before sending data.
		UINT32 bufferSize = m_nSendBufferSize;
		UINT32 times = (size / bufferSize) + 1;
		UINT32 last = size % bufferSize;

		for (UINT32 index = 0; index < times; index++)
		{
			OverlappedWrapper& buffer = OverlappedWrapper::NewBuffer(connection, AsyncOperation::Send, bufferSize);
			if (index == times - 1)
			{
				memcpy_s(buffer.wsaBuffer.buf, buffer.wsaBuffer.len, data + offset, last);
				buffer.wsaBuffer.len = last;
				offset += last;
			}
			else
			{
				memcpy_s(buffer.wsaBuffer.buf, buffer.wsaBuffer.len, data + offset, bufferSize);
				buffer.wsaBuffer.len = bufferSize;
				offset = index * bufferSize;
			}
			if (SOCKET_ERROR == WSASend(connection->Socket(), &(buffer.wsaBuffer), 1, &dwBytes, dwFlags, &buffer.overlapped, NULL))
			{
				UINT64 errcode = buffer.GetErrorCode();
				OnError(connection->Connection(), errcode);
				buffer.owner = NULL;
				return;
			}
		}
	}

	return;
}

void IOCPCommunication::Remove(const IConnectionBase* pConnection)
{
	//LOCK;

	if (pConnection == NULL)
	{
		OnError(pConnection, ERROR_INVALID_CONNECTION);
		return;
	}

	AsyncConnectionWrapperPtr asyncConn = AsyncConnectionWrapper::Find(pConnection);

	if (asyncConn == NULL)
	{
		OnError(pConnection, ERROR_DISCONNECTED_CONNECTION);
		return;
	}

	//Close client socket...
	asyncConn->Close(g_lpfnDisconnectEx);
	return;
}

void IOCPCommunication::Shutdown()
{

	for (UINT32 index = 0; index < m_nCountOfThreads; index++)
	{
		::PostQueuedCompletionStatus(m_hCompletionPort, -1, NULL, NULL);
	}
}