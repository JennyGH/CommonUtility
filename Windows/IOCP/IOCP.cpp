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

        // ����յ������˳���־����ֱ���˳�
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

        // �ж��Ƿ��пͻ��˶Ͽ���
        if ((0 == dwBytesTransfered) &&
            (OverlappedOpType::Recv == pIOContext->opType || OverlappedOpType::Send == pIOContext->opType))
        {
            // �ͷŵ���Ӧ����Դ
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
                // �ͷ�IO������
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
                //TRACE(_T("_WorkThread�е� pIoContext->m_OpType �����쳣.\n"));
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

    // �����û�����ʱ�Ŵ����µ����Ӷ���
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

    // ����������ϣ������ Socket ����ɶ˿ڰ�(��Ҳ��һ���ؼ�����)
    HANDLE hTemp = ::CreateIoCompletionPort(
        (HANDLE)pAcceptSocketContext->socket,
        this->m_hCompletionPort,
        (ULONG_PTR)pAcceptSocketContext, 0);
    if (NULL == hTemp)
    {
        delete pAcceptSocketContext;
        OnError(NULL, WSAGetLastError());
        // �ͷžɵ�IO������
        pListenSocketContext->RemoveContext(pOldIOContext);
        return FALSE;
    }

    // �����֮�󣬾Ϳ��Կ�ʼ����� Socket ��Ͷ�����������
    if (FALSE == this->_PostRecv(pAcceptSocketContext))
    {
        // �ͷžɵ�IO������
        pListenSocketContext->RemoveContext(pOldIOContext);
        return FALSE;
    }

    // Ͷ�� Recv �ɹ�������ǰ�û���ӵ��б���
    ((CriticalSection*)m_hCriticalSection)->Lock();
    this->m_clientSocketContexts[pConnection] = pAcceptSocketContext;
    ((CriticalSection*)m_hCriticalSection)->UnLock();

    // �ص�֪ͨ�ϲ�
    OnAccepted(pConnection, (CONST BYTE*)pOldIOContext->wsaBuf.buf, dwBytes);

    // �ͷžɵ�IO������
    pListenSocketContext->RemoveContext(pOldIOContext);

    // ��Ͷ��һ���µ� Accept ����
    return this->_PostAccept();
}

BOOL IOCP::_PostAccept()
{
    DWORD dwBytes = 0;
    // Ϊ�Ժ�������Ŀͻ�����׼���� Socket
    SOCKET acceptSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == acceptSocket)
    {
        // ��������Accept��Socketʧ��
        OnError(NULL, WSAGetLastError());
        return FALSE;
    }

    PER_SOCKET_CONTEXT_PTR pListenSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(m_hListenSocketContext);
    PER_IO_CONTEXT_PTR pNewIOContext = pListenSocketContext->NewIOContext(OverlappedOpType::Accept);
    pNewIOContext->opSocket = acceptSocket;

    // Ͷ�� AcceptEx
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
            // Ͷ�� AcceptEx ����ʧ��
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
    // �Ȱ���һ�ε�������ʾ���֣�Ȼ�������״̬��������һ�� Recv ����
    OnRecved((IConnectionBase*)pSocketContext->pConnection, (CONST BYTE*)pOldIOContext->wsaBuf.buf, dwBytes);
    // �ͷžɵ�IO������
    pSocketContext->RemoveContext(pOldIOContext);
    // Ȼ��ʼͶ����һ��WSARecv����
    return _PostRecv(pSocketContext);
}

BOOL IOCP::_PostRecv(HANDLE hSocketContext)
{
    PER_SOCKET_CONTEXT_PTR pClientSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(hSocketContext);
    DWORD dwFlags = 0;
    DWORD dwBytes = 0;

    // ����һ���µ�IO������
    PER_IO_CONTEXT_PTR pNewIOContext = pClientSocketContext->NewIOContext(OverlappedOpType::Recv);
    // ��ʼ����ɺ�Ͷ�� WSARecv ����
    INT32 nBytesRecv = ::WSARecv(pClientSocketContext->socket, &pNewIOContext->wsaBuf, 1, &dwBytes, &dwFlags, &pNewIOContext->overlapped, NULL);
    // �������ֵ���󣬲��Ҵ���Ĵ��벢���� Pending �Ļ����Ǿ�˵������ص�����ʧ����
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
    // ��ӳ������Ƴ�
    this->m_clientSocketContexts.erase(pConnection);
    ((CriticalSection*)this->m_hCriticalSection)->UnLock();
    // �ͷ���������Ϣ
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

    //������ɶ˿�
    ::CreateIoCompletionPort((HANDLE)pListenSocketContext->socket, m_hCompletionPort, (ULONG_PTR)pListenSocketContext, 0);
    if (SOCKET_ERROR == ::listen(pListenSocketContext->socket, SOMAXCONN))
    {
        OnError(NULL, WSAGetLastError());
        return;
    }
    //�������ɸ������߳�
    {
        thread_t thread = { _WorkerThread, this };
        ThreadHelper::Create(thread, m_nCountOfThreads);
    }

    // ΪAcceptEx ׼��������Ȼ��Ͷ��AcceptEx I/O����
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
    // ��ӳ������ҵ����Ӷ����Ӧ����������Ϣ
    ((CriticalSection*)this->m_hCriticalSection)->Lock();
    PER_SOCKET_CONTEXT_PTR pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(this->m_clientSocketContexts[pConnection]);
    ((CriticalSection*)this->m_hCriticalSection)->UnLock();
    if (NULL == pSocketContext)
    {
        OnError(pConnection, ERROR_INVALID_HANDLE);
        return;
    }
    // ����һ��IO������
    PER_IO_CONTEXT_PTR pNewIOContext = pSocketContext->NewIOContext(OverlappedOpType::Send);
    // ��Ҫ���͵����ݿ�����IO�����Ļ�����
    memcpy_s(pNewIOContext->wsaBuf.buf, pNewIOContext->wsaBuf.len, data, size);
    // Ͷ��һ�� Send ����
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
        // �Ƚ�������ָ���ó���
        pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(found->second);
        // �ٴ�ӳ������Ƴ���ǰ���Ӷ������Ϣ
        this->m_clientSocketContexts.erase(pConnection);
    }
    ((CriticalSection*)this->m_hCriticalSection)->UnLock();
    // ֪ͨ�ϲ�
    OnRemoved(pConnection);
    // �ͷ���������Ϣ
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
    // �ͷ����� socket context
    std::map<IConnectionBase*, HANDLE>::iterator iter = this->m_clientSocketContexts.begin();
    std::map<IConnectionBase*, HANDLE>::iterator end = this->m_clientSocketContexts.end();
    for (iter; iter != end; iter++)
    {
        PER_SOCKET_CONTEXT_PTR pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(iter->second);
        // �ͷ���������Ϣ
        delete pSocketContext;
        iter->second = NULL;
        // ֪ͨ�ϲ�
        OnRemoved(iter->first);
    }
    for (UINT32 index = 0; index < m_nCountOfThreads; index++)
    {
        ::PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
    }
}