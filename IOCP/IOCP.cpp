#include "stdafx.h"
#include "IOCP.h"
#include "Types.h"
#include "ThreadHelper.h"
#include "CriticalSection.h"

#define EXIT_CODE                       0xFFFFFFFF
#define MAX_POST_ACCEPT                 10
#define WORKER_THREADS_PER_PROCESSOR    2

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

IOCP::IOCP(IConnectionFactory *pConnectionFactory, const IOCPSettings& settings) :
    m_nAF(settings.af),
    m_nProtocol(settings.protocol),
    m_nSendBufferSize(settings.nSendBufferSize),
    m_nRecvBufferSize(settings.nRecvBufferSize <= 0 ? DEFAULT_RECV_BUFFER_SIZE : settings.nRecvBufferSize),
    m_hListenContext(NULL),
    m_pAddress(NULL),
    m_hCompletionPort(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)),
    m_hCriticalSection(new CriticalSection()),
    m_nCountOfThreads(settings.nCountOfThreads > 0 ? settings.nCountOfThreads : GetCountOfProcessors() * WORKER_THREADS_PER_PROCESSOR),
    m_pFactory(pConnectionFactory),
    m_isAcceptWithData(settings.isAcceptWithData)
{
    WSAData wsaData;
    if (SOCKET_ERROR == WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }
    SOCKET socket = INVALID_SOCKET;
    if (INVALID_SOCKET == (socket = WSASocket(m_nAF, SOCK_STREAM, m_nProtocol, NULL, 0, WSA_FLAG_OVERLAPPED)))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }
    m_hListenContext = new PER_SOCKET_CONTEXT(socket, NULL);
    if (SOCKET_ERROR == GetFunction(socket, g_guidAcceptEx, &g_lpfnAcceptEx))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }
    if (SOCKET_ERROR == GetFunction(socket, g_guidDisconnectEx, &g_lpfnDisconnectEx))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }
    if (SOCKET_ERROR == GetFunction(socket, g_guidGetAcceptExSockaddrs, &g_lpfnGetAcceptExSockaddrs))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }
}

IOCP::~IOCP()
{
    Shutdown();

    ::CloseHandle(m_hCompletionPort);

    delete ((PER_SOCKET_CONTEXT_PTR)m_hListenContext);
    delete ((IConnectionFactory*)m_pFactory);
    delete ((CriticalSection*)m_hCriticalSection);

    WSACleanup();
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

unsigned WINAPI IOCP::_WorkerThread(HANDLE hHandle)
{
    if (hHandle == NULL)
    {
        return ERROR_BAD_ARGUMENTS;
    }

    IOCP& self = *(IOCP*)hHandle;

    while (true)
    {
        DWORD dwBytesTransfered = 0;
        OVERLAPPED* pOverlapped = NULL;
        PER_SOCKET_CONTEXT_PTR pSocketContext = NULL;
        BOOL bSuccess = ::GetQueuedCompletionStatus(
            self.m_hCompletionPort,
            &dwBytesTransfered,
            (PULONG_PTR)&pSocketContext,
            (LPOVERLAPPED*)&pOverlapped,
            WSA_INFINITE);

        // 如果收到的是退出标志，则直接退出
        if (EXIT_CODE == (DWORD)pSocketContext)
        {
            //::SetEvent(self.m_hShutdownEvent);
            return 0; // EXIT !!!
        }

        if (!bSuccess)
        {
            self.OnError(NULL, WSAGetLastError());
            continue;
        }

        PER_IO_CONTEXT_PTR pIOContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, overlapped);

        // 判断是否有客户端断开了
        if ((0 == dwBytesTransfered) &&
            (OverlappedOpType::Recv == pIOContext->opType || OverlappedOpType::Send == pIOContext->opType))
        {
            // 释放掉对应的资源
            // Remove client socket context.
            self.OnClientLost((IConnectionBase*)pSocketContext->pConnection);
            self._RemoveClientSocketContext(pSocketContext);
            continue;
        }
        else
        {
            switch (pIOContext->opType)
            {
            case OverlappedOpType::Accept:
            {
                self._DoAccept(pSocketContext, pIOContext, dwBytesTransfered);
                break;
            }
            case OverlappedOpType::Recv:
            {
                self._DoRecv(pSocketContext, pIOContext, dwBytesTransfered);
                break;
            }
            case OverlappedOpType::Send:
            {
                self.OnSent(
                    (IConnectionBase*)pSocketContext->pConnection,
                    (CONST BYTE*)pIOContext->wsaBuf.buf,
                    pIOContext->wsaBuf.len
                );
                break;
            }
            default:
                self.OnError((IConnectionBase*)pSocketContext->pConnection, WSAGetLastError());
                //TRACE(_T("_WorkThread中的 pIoContext->m_OpType 参数异常.\n"));
                break;
            }
        }
    }

    return 0;
}

BOOL IOCP::_DoAccept(HANDLE hSocketContext, HANDLE hIOContext, DWORD dwBytes)
{
    PER_SOCKET_CONTEXT_PTR pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(hSocketContext);
    PER_IO_CONTEXT_PTR pIOContext = static_cast<PER_IO_CONTEXT_PTR>(hIOContext);
    PER_SOCKET_CONTEXT_PTR pListenSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(this->m_hListenContext);

    // 有新用户连接时才创建新的连接对象
    IConnectionBase* pConnection = m_pFactory->NewConnection();
    pSocketContext->pConnection = pConnection;
    SOCKADDR_IN* pRemoteAddr = NULL;
    SOCKADDR_IN* pLocaleAddr = NULL;
    INT32 nRemoteAddrSize = sizeof(SOCKADDR_IN);
    INT32 nLocaleAddrSize = sizeof(SOCKADDR_IN);

    g_lpfnGetAcceptExSockaddrs(
        pIOContext->wsaBuf.buf,
        pIOContext->wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
        sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16,
        (LPSOCKADDR*)&pLocaleAddr,
        &nLocaleAddrSize,
        (LPSOCKADDR*)&pRemoteAddr,
        &nRemoteAddrSize);

    pConnection->SetAddress(pRemoteAddr, nRemoteAddrSize);

    PER_SOCKET_CONTEXT_PTR pNewSocketContext = new PER_SOCKET_CONTEXT(pIOContext->opSocket, NULL);

    // 参数设置完毕，将这个 Socket 和完成端口绑定(这也是一个关键步骤)
    HANDLE hTemp = ::CreateIoCompletionPort(
        (HANDLE)pNewSocketContext->socket,
        this->m_hCompletionPort,
        (DWORD)pNewSocketContext, 0);
    if (NULL == hTemp)
    {
        delete pNewSocketContext;
        OnError(NULL, WSAGetLastError());
        return FALSE;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // 3. 继续，建立其下的IoContext，用于在这个Socket上投递第一个Recv数据请求
    PER_IO_CONTEXT_PTR pNewIoContext = pSocketContext->NewIOContext(OverlappedOpType::Recv);

    // 绑定完毕之后，就可以开始在这个 Socket 上投递完成请求了
    if (FALSE == this->_PostRecv(pNewIoContext))
    {
        pSocketContext->RemoveContext(pNewIoContext);
        return FALSE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // 4. 如果投递成功，那么就把这个有效的客户端信息，加入到ContextList中去(需要统一管理，方便释放资源)
    ((CriticalSection*)m_hCriticalSection)->Lock();
    this->m_clientSocketContexts[pConnection] = pSocketContext;
    ((CriticalSection*)m_hCriticalSection)->UnLock();

    // 回调通知上层
    OnAccepted(pConnection, (CONST BYTE*)pIOContext->wsaBuf.buf, dwBytes);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // 5. 使用完毕之后，把 Listen Socket 的那个 IoContext 重置，然后准备投递新的 AcceptEx
    pIOContext->ResetBuffer();

    return this->_PostAccept(pIOContext);
}

BOOL IOCP::_PostAccept(HANDLE hHandle)
{
    PER_IO_CONTEXT_PTR pAcceptIOContext = static_cast<PER_IO_CONTEXT_PTR>(hHandle);
    PER_SOCKET_CONTEXT_PTR pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(this->m_hListenContext);

    //::ASSERT(INVALID_SOCKET != pSocketContext->socket);

    // 准备参数
    pAcceptIOContext->opType = OverlappedOpType::Accept;
    DWORD dwBytes = 0;
    WSABUF *pWSABuffer = &pAcceptIOContext->wsaBuf;
    OVERLAPPED *pOverlapped = &pAcceptIOContext->overlapped;

    // 为以后新连入的客户端先准备好 Socket
    pAcceptIOContext->opSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == pAcceptIOContext->opSocket)
    {
        // 创建用于Accept的Socket失败
        OnError(NULL, WSAGetLastError());
        return FALSE;
    }

    // 投递 AcceptEx
    if (FALSE == g_lpfnAcceptEx(
        pSocketContext->socket,
        pAcceptIOContext->opSocket,
        pWSABuffer->buf,
        pWSABuffer->len - ((sizeof(SOCKADDR_IN) + 16) * 2),
        sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16,
        &dwBytes, pOverlapped))
    {
        int err = WSAGetLastError();
        if (WSA_IO_PENDING != err)
        {
            // 投递 AcceptEx 请求失败
            OnError(NULL, err);
            return FALSE;
        }
    }

    return TRUE;
}

BOOL IOCP::_DoRecv(HANDLE hSocketContext, HANDLE hIOContext, DWORD dwBytes)
{
    PER_SOCKET_CONTEXT_PTR pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(hSocketContext);
    PER_IO_CONTEXT_PTR pIOContext = static_cast<PER_IO_CONTEXT_PTR>(hIOContext);
    PER_SOCKET_CONTEXT_PTR pListenSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(this->m_hListenContext);
    // 先把上一次的数据显示出现，然后就重置状态，发出下一个 Recv 请求
    OnRecved((IConnectionBase*)pSocketContext->pConnection, (CONST BYTE*)pIOContext->wsaBuf.buf, dwBytes);
    // 然后开始投递下一个WSARecv请求
    return _PostRecv(pIOContext);
}

BOOL IOCP::_PostRecv(HANDLE hHandle)
{
    PER_IO_CONTEXT_PTR pIOContext = static_cast<PER_IO_CONTEXT_PTR>(hHandle);
    PER_SOCKET_CONTEXT_PTR pListenSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(this->m_hListenContext);
    // 初始化变量
    DWORD dwFlags = 0;
    DWORD dwBytes = 0;
    WSABUF *pWSABuffer = &pIOContext->wsaBuf;
    OVERLAPPED *pOverlapped = &pIOContext->overlapped;

    pIOContext->ResetBuffer();
    pIOContext->opType = OverlappedOpType::Recv;

    // 初始化完成后，投递 WSARecv 请求
    INT32 nBytesRecv = ::WSARecv(pIOContext->opSocket, pWSABuffer, 1, &dwBytes, &dwFlags, pOverlapped, NULL);

    // 如果返回值错误，并且错误的代码并非是 Pending 的话，那就说明这个重叠请求失败了
    if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
    {
        OnError(NULL, WSAGetLastError());
        return FALSE;
    }
    return TRUE;
}

VOID IOCP::_RemoveClientSocketContext(HANDLE hSocketContext)
{
    ((CriticalSection*)this->m_hCriticalSection)->Lock();
    PER_SOCKET_CONTEXT_PTR pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(hSocketContext);
    IConnectionBase* pConnection = (IConnectionBase*)(pSocketContext->pConnection);
    // 从映射表中移除
    this->m_clientSocketContexts.erase(pConnection);
    // 释放上下文信息
    delete pSocketContext;
    ((CriticalSection*)this->m_hCriticalSection)->UnLock();
    // 通知上层
    OnRemoved(pConnection);
}

void IOCP::Start(const void * addr, UINT32 sizeOfAddress)
{
    if (addr == NULL)
    {
        OnError(NULL, ERROR_BAD_ARGUMENTS);
        return;
    }
    if (m_pFactory == NULL)
    {
        OnError(NULL, ERROR_INVALID_HANDLE);
        return;
    }
    PER_SOCKET_CONTEXT_PTR hSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(this->m_hListenContext);
    sockaddr* address = (sockaddr*)addr;
    address->sa_family = m_nAF;
    if (SOCKET_ERROR == ::bind(hSocketContext->socket, address, sizeOfAddress))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }

    //创建完成端口
    ::CreateIoCompletionPort((HANDLE)hSocketContext->socket, m_hCompletionPort, (DWORD)hSocketContext, 0);
    if (SOCKET_ERROR == ::listen(hSocketContext->socket, SOMAXCONN))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }
    //创建若干个工作线程
    {
        thread_t thread = { _WorkerThread, this };
        ThreadHelper::Create(thread, m_nCountOfThreads);
    }

    // 为AcceptEx 准备参数，然后投递AcceptEx I/O请求
    PER_SOCKET_CONTEXT_PTR hListenContext = static_cast<PER_SOCKET_CONTEXT_PTR>(m_hListenContext);
    for (int i = 0; i < MAX_POST_ACCEPT; i++)
    {
        // 新建一个IO_CONTEXT
        PER_IO_CONTEXT* pAcceptIoContext = hListenContext->NewIOContext(OverlappedOpType::Accept);
        if (FALSE == this->_PostAccept(pAcceptIoContext))
        {
            hListenContext->RemoveContext(pAcceptIoContext);
            return;
        }
    }
}
void IOCP::Send(IConnectionBase* pConnection, const BYTE data[], UINT32 size)
{
    DWORD dwBytes = 0;
    DWORD dwFlags = 0;
    UINT32 offset = 0;
    if (NULL == pConnection)
    {
        OnError(pConnection, ERROR_INVALID_HANDLE);
        return;
    }
    // 从映射表中找到连接对象对应的上下文信息
    ((CriticalSection*)this->m_hCriticalSection)->Lock();
    PER_SOCKET_CONTEXT_PTR pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(this->m_clientSocketContexts[pConnection]);
    ((CriticalSection*)this->m_hCriticalSection)->UnLock();
    if (NULL == pSocketContext)
    {
        OnError(pConnection, ERROR_INVALID_HANDLE);
        return;
    }
    PER_IO_CONTEXT_PTR pIOContext = pSocketContext->NewIOContext(OverlappedOpType::Send);
    memcpy_s(pIOContext->wsaBuf.buf, pIOContext->wsaBuf.len, data, size);
    if (SOCKET_ERROR == ::WSASend(pSocketContext->socket, &pIOContext->wsaBuf, 1, &dwBytes, dwFlags, &pIOContext->overlapped, NULL))
    {
        int err = ::WSAGetLastError();
        if (WSA_IO_PENDING != err)
        {
            OnError(pConnection, err);
        }
        return;
    }
}

void IOCP::Remove(IConnectionBase* pConnection)
{
    ((CriticalSection*)this->m_hCriticalSection)->Lock();
    PER_SOCKET_CONTEXT_PTR pSocketContext = NULL;
    std::map<IConnectionBase*, HANDLE>::iterator found = this->m_clientSocketContexts.find(pConnection);
    if (found != this->m_clientSocketContexts.end())
    {
        // 先将上下文指针拿出来
        pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(found->second);
        // 再从映射表中移除当前连接对象的信息
        this->m_clientSocketContexts.erase(pConnection);
        // 释放上下文信息
        delete pSocketContext;
    }
    ((CriticalSection*)this->m_hCriticalSection)->UnLock();
    // 通知上层
    OnRemoved(pConnection);
    return;
}

void IOCP::Shutdown()
{
    // 释放所有 socket context
    std::map<IConnectionBase*, HANDLE>::iterator iter = this->m_clientSocketContexts.begin();
    std::map<IConnectionBase*, HANDLE>::iterator end = this->m_clientSocketContexts.end();
    for (iter; iter != end; iter++)
    {
        PER_SOCKET_CONTEXT_PTR pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(iter->second);
        // 释放上下文信息
        delete pSocketContext;
        iter->second = NULL;
        // 通知上层
        OnRemoved(iter->first);
    }
    for (UINT32 index = 0; index < m_nCountOfThreads; index++)
    {
        ::PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
    }
}