#include "stdafx.h"
#include "IOCP.h"
#include "Types.h"
#include "ThreadHelper.h"
#include "CriticalSection.h"

#define EXIT_CODE                       0xFFFFFFFF
#define WORKER_THREADS_PER_PROCESSOR    2
#define LOG_DEBUG(fmt, ...)             printf(fmt " \n", ##__VA_ARGS__)

#define LOCK \
((CriticalSection*)m_hCriticalSection)->Lock();\
common::raii::scope_member_function_noparam<CriticalSection, void> scope_lock((CriticalSection*)m_hCriticalSection, &CriticalSection::UnLock)

static GUID                                     g_guidAcceptEx = WSAID_ACCEPTEX;
static GUID                                     g_guidDisconnectEx = WSAID_DISCONNECTEX;
static GUID                                     g_guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
static LPFN_ACCEPTEX                            g_lpfnAcceptEx = NULL;
static LPFN_DISCONNECTEX                        g_lpfnDisconnectEx = NULL;
static LPFN_GETACCEPTEXSOCKADDRS                g_lpfnGetAcceptExSockaddrs = NULL;
static UINT32                                   GetCountOfProcessors();
static INT32                                    GetFunctionByGuid(UINT_PTR socket, GUID& guid, LPVOID lpFn, DWORD nSizeOfFn);
#define GetFunction(socket, guid, lpFn)         GetFunctionByGuid(socket, guid, lpFn, sizeof(lpFn))

IOCP::IOCP(IConnectionFactory* pConnectionFactory, const IOCPSettings& settings) :
    m_nAF(settings.af),
    m_nProtocol(settings.protocol),
    m_nSendBufferSize(settings.nSendBufferSize),
    m_nRecvBufferSize(settings.nRecvBufferSize <= 0 ? DEFAULT_RECV_BUFFER_SIZE : settings.nRecvBufferSize),
    m_hListenSocketContext(NULL),
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
    SOCKET listenSocket = INVALID_SOCKET;
    if (INVALID_SOCKET == (listenSocket = WSASocket(m_nAF, SOCK_STREAM, m_nProtocol, NULL, 0, WSA_FLAG_OVERLAPPED)))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }
    m_hListenSocketContext = new PER_SOCKET_CONTEXT(listenSocket);
    if (SOCKET_ERROR == GetFunction(listenSocket, g_guidAcceptEx, &g_lpfnAcceptEx))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }
    if (SOCKET_ERROR == GetFunction(listenSocket, g_guidDisconnectEx, &g_lpfnDisconnectEx))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }
    if (SOCKET_ERROR == GetFunction(listenSocket, g_guidGetAcceptExSockaddrs, &g_lpfnGetAcceptExSockaddrs))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }
}

IOCP::~IOCP()
{
    Shutdown();

    ::CloseHandle(m_hCompletionPort);

    delete ((PER_SOCKET_CONTEXT_PTR)m_hListenSocketContext);
    delete ((IConnectionFactory*)m_pFactory);
    delete ((CriticalSection*)m_hCriticalSection);

    WSACleanup();
}

static INT32 GetFunctionByGuid(UINT_PTR socket, GUID& guid, LPVOID lpFn, DWORD nSizeOfFn)
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
        IConnectionBase* pConnection = static_cast<IConnectionBase*>(pSocketContext->pConnection);

        // 判断是否有客户端断开了
        if ((0 == dwBytesTransfered) &&
            (OverlappedOpType::Recv == pIOContext->opType || OverlappedOpType::Send == pIOContext->opType))
        {
            // 释放掉对应的资源
            // Remove client socket context.
            self.OnClientLost(pConnection);
            self._RemoveClientSocketContext(pSocketContext);
            continue;
        }
        else
        {
            switch (pIOContext->opType)
            {
            case OverlappedOpType::Accept:
            {
                self._DoAccept(pIOContext, dwBytesTransfered);
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
                    pConnection,
                    (CONST BYTE*)pIOContext->wsaBuf.buf,
                    pIOContext->wsaBuf.len
                );
                // 释放IO上下文
                pSocketContext->RemoveContext(pIOContext);
                break;
            }
            case OverlappedOpType::Remove:
            {
                LOG_DEBUG("Remove user, pSocketContext: %s, count of PER_IO_CONTEXT: %d", pConnection->GetAddress().c_str(), pSocketContext->GetIOContextCount());
                delete pSocketContext;
                break;
            }
            case OverlappedOpType::Disconnect:
            {
                LOG_DEBUG("User disconnect, pSocketContext: %s, count of PER_IO_CONTEXT: %d", pConnection->GetAddress().c_str(), pSocketContext->GetIOContextCount());
                delete pSocketContext;
                break;
            }
            default:
                self.OnError(pConnection, WSAGetLastError());
                //TRACE(_T("_WorkThread中的 pIoContext->m_OpType 参数异常.\n"));
                break;
            }
        }
    }

    return 0;
}

BOOL IOCP::_DoAccept(HANDLE hIOContext, DWORD dwBytes)
{
    PER_IO_CONTEXT_PTR pOldIOContext = static_cast<PER_IO_CONTEXT_PTR>(hIOContext);
    PER_SOCKET_CONTEXT_PTR pAcceptSocketContext = new PER_SOCKET_CONTEXT(pOldIOContext->opSocket);
    PER_SOCKET_CONTEXT_PTR pListenSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(m_hListenSocketContext);

    // 有新用户连接时才创建新的连接对象
    IConnectionBase* pConnection = m_pFactory->NewConnection();
    pAcceptSocketContext->pConnection = pConnection;
    SOCKADDR* pRemoteAddr = NULL;
    SOCKADDR* pLocaleAddr = NULL;
    INT32 nRemoteAddrSize = 0;
    INT32 nLocaleAddrSize = 0;

    g_lpfnGetAcceptExSockaddrs(
        pOldIOContext->wsaBuf.buf,
        m_isAcceptWithData ? (pOldIOContext->wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2)) : 0,
        sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16,
        &pLocaleAddr,
        &nLocaleAddrSize,
        &pRemoteAddr,
        &nRemoteAddrSize);

    pConnection->SetAddress(pRemoteAddr, nRemoteAddrSize);

    //Set socket options.
    {
        LINGER lingerStruct;
        lingerStruct.l_onoff = 1;
        lingerStruct.l_linger = 0;
        ::setsockopt(
            pAcceptSocketContext->socket,
            SOL_SOCKET,
            SO_UPDATE_ACCEPT_CONTEXT,
            (char*)&(pListenSocketContext->socket),
            sizeof(pListenSocketContext->socket)
        );
        ::setsockopt(
            pAcceptSocketContext->socket,
            SOL_SOCKET,
            SO_LINGER,
            (char *)&lingerStruct,
            sizeof(lingerStruct)
        );
    }

    // 参数设置完毕，将这个 Socket 和完成端口绑定(这也是一个关键步骤)
    HANDLE hTemp = ::CreateIoCompletionPort(
        (HANDLE)pAcceptSocketContext->socket,
        this->m_hCompletionPort,
        (ULONG_PTR)pAcceptSocketContext, 0);
    if (NULL == hTemp)
    {
        delete pAcceptSocketContext;
        OnError(NULL, WSAGetLastError());
        // 释放旧的IO上下文
        pListenSocketContext->RemoveContext(pOldIOContext);
        return FALSE;
    }

    // 绑定完毕之后，就可以开始在这个 Socket 上投递完成请求了
    if (FALSE == this->_PostRecv(pAcceptSocketContext))
    {
        // 释放旧的IO上下文
        pListenSocketContext->RemoveContext(pOldIOContext);
        return FALSE;
    }

    // 投递 Recv 成功，将当前用户添加到列表中
    ((CriticalSection*)m_hCriticalSection)->Lock();
    this->m_clientSocketContexts[pConnection] = pAcceptSocketContext;
    ((CriticalSection*)m_hCriticalSection)->UnLock();

    // 回调通知上层
    OnAccepted(pConnection, (CONST BYTE*)pOldIOContext->wsaBuf.buf, dwBytes);

    // 释放旧的IO上下文
    pListenSocketContext->RemoveContext(pOldIOContext);

    // 再投递一个新的 Accept 请求
    return this->_PostAccept();
}

BOOL IOCP::_PostAccept()
{
    DWORD dwBytes = 0;
    // 为以后新连入的客户端先准备好 Socket
    SOCKET acceptSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == acceptSocket)
    {
        // 创建用于Accept的Socket失败
        OnError(NULL, WSAGetLastError());
        return FALSE;
    }

    PER_SOCKET_CONTEXT_PTR pListenSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(m_hListenSocketContext);
    PER_IO_CONTEXT_PTR pNewIOContext = pListenSocketContext->NewIOContext(OverlappedOpType::Accept);
    pNewIOContext->opSocket = acceptSocket;

    // 投递 AcceptEx
    if (FALSE == g_lpfnAcceptEx(
        pListenSocketContext->socket,
        acceptSocket,
        pNewIOContext->wsaBuf.buf,
        m_isAcceptWithData ? pNewIOContext->wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2) : 0,
        sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16,
        &dwBytes,
        &pNewIOContext->overlapped))
    {
        int err = WSAGetLastError();
        if (WSA_IO_PENDING != err)
        {
            // 投递 AcceptEx 请求失败
            OnError(NULL, err);
            pListenSocketContext->RemoveContext(pNewIOContext);
            return FALSE;
        }
    }

    return TRUE;
}

BOOL IOCP::_DoRecv(HANDLE hSocketContext, HANDLE hIOContext, DWORD dwBytes)
{
    PER_SOCKET_CONTEXT_PTR pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(hSocketContext);
    PER_IO_CONTEXT_PTR pOldIOContext = static_cast<PER_IO_CONTEXT_PTR>(hIOContext);
    // 先把上一次的数据显示出现，然后就重置状态，发出下一个 Recv 请求
    OnRecved((IConnectionBase*)pSocketContext->pConnection, (CONST BYTE*)pOldIOContext->wsaBuf.buf, dwBytes);
    // 释放旧的IO上下文
    pSocketContext->RemoveContext(pOldIOContext);
    // 然后开始投递下一个WSARecv请求
    return _PostRecv(pSocketContext);
}

BOOL IOCP::_PostRecv(HANDLE hSocketContext)
{
    PER_SOCKET_CONTEXT_PTR pClientSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(hSocketContext);
    DWORD dwFlags = 0;
    DWORD dwBytes = 0;

    // 创建一个新的IO上下文
    PER_IO_CONTEXT_PTR pNewIOContext = pClientSocketContext->NewIOContext(OverlappedOpType::Recv);
    // 初始化完成后，投递 WSARecv 请求
    INT32 nBytesRecv = ::WSARecv(pClientSocketContext->socket, &pNewIOContext->wsaBuf, 1, &dwBytes, &dwFlags, &pNewIOContext->overlapped, NULL);
    // 如果返回值错误，并且错误的代码并非是 Pending 的话，那就说明这个重叠请求失败了
    if ((SOCKET_ERROR == nBytesRecv))
    {
        int err = WSAGetLastError();
        if (WSA_IO_PENDING != err)
        {
            OnError((IConnectionBase*)pClientSocketContext->pConnection, err);
            pClientSocketContext->RemoveContext(pNewIOContext);
            return FALSE;
        }
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
    ((CriticalSection*)this->m_hCriticalSection)->UnLock();
    // 释放上下文信息
    if (NULL != pSocketContext)
    {
        //PER_IO_CONTEXT_PTR pIOContext = pSocketContext->NewIOContext(OverlappedOpType::Disconnect);
        //BOOL bSuccess = g_lpfnDisconnectEx(pIOContext->opSocket, &pIOContext->overlapped, 0, 0);
        //if (!bSuccess)
        //{
        //    int err = WSAGetLastError();
        //    if (WSA_IO_PENDING != err)
        //    {
        //        OnError(pConnection, err);
        //    }
        //}

        delete pSocketContext;
    }
}

void IOCP::Start(const void* addr, UINT32 sizeOfAddress)
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
    sockaddr* address = (sockaddr*)addr;
    address->sa_family = m_nAF;
    PER_SOCKET_CONTEXT_PTR pListenSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(m_hListenSocketContext);
    if (SOCKET_ERROR == ::bind(pListenSocketContext->socket, address, sizeOfAddress))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }

    //创建完成端口
    ::CreateIoCompletionPort((HANDLE)pListenSocketContext->socket, m_hCompletionPort, (ULONG_PTR)pListenSocketContext, 0);
    if (SOCKET_ERROR == ::listen(pListenSocketContext->socket, SOMAXCONN))
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
    for (int i = 0; i < m_nCountOfThreads; i++)
    {
        this->_PostAccept();
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
    // 创建一个IO上下文
    PER_IO_CONTEXT_PTR pNewIOContext = pSocketContext->NewIOContext(OverlappedOpType::Send);
    // 将要发送的数据拷贝到IO上下文缓冲区
    memcpy_s(pNewIOContext->wsaBuf.buf, pNewIOContext->wsaBuf.len, data, size);
    // 投递一个 Send 请求
    if (SOCKET_ERROR == ::WSASend(pSocketContext->socket, &pNewIOContext->wsaBuf, 1, &dwBytes, dwFlags, &pNewIOContext->overlapped, NULL))
    {
        int err = ::WSAGetLastError();
        if (WSA_IO_PENDING != err)
        {
            OnError(pConnection, err);
            pSocketContext->RemoveContext(pNewIOContext);
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
    }
    ((CriticalSection*)this->m_hCriticalSection)->UnLock();
    // 通知上层
    OnRemoved(pConnection);
    // 释放上下文信息
    if (NULL != pSocketContext)
    {
        //PER_IO_CONTEXT_PTR pIOContext = pSocketContext->NewIOContext(OverlappedOpType::Remove);
        //BOOL bSuccess = g_lpfnDisconnectEx(pIOContext->opSocket, &pIOContext->overlapped, 0, 0);
        //if (!bSuccess)
        //{
        //    int err = WSAGetLastError();
        //    if (WSA_IO_PENDING != err)
        //    {
        //        OnError(pConnection, err);
        //    }
        //}
        delete pSocketContext;
    }
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