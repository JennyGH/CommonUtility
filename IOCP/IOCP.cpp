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

        // �ж��Ƿ��пͻ��˶Ͽ���
        if ((0 == dwBytesTransfered) &&
            (OverlappedOpType::Recv == pIOContext->opType || OverlappedOpType::Send == pIOContext->opType))
        {
            // �ͷŵ���Ӧ����Դ
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
                //TRACE(_T("_WorkThread�е� pIoContext->m_OpType �����쳣.\n"));
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

    // �����û�����ʱ�Ŵ����µ����Ӷ���
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

    // ����������ϣ������ Socket ����ɶ˿ڰ�(��Ҳ��һ���ؼ�����)
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
    // 3. �������������µ�IoContext�����������Socket��Ͷ�ݵ�һ��Recv��������
    PER_IO_CONTEXT_PTR pNewIoContext = pSocketContext->NewIOContext(OverlappedOpType::Recv);

    // �����֮�󣬾Ϳ��Կ�ʼ����� Socket ��Ͷ�����������
    if (FALSE == this->_PostRecv(pNewIoContext))
    {
        pSocketContext->RemoveContext(pNewIoContext);
        return FALSE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // 4. ���Ͷ�ݳɹ�����ô�Ͱ������Ч�Ŀͻ�����Ϣ�����뵽ContextList��ȥ(��Ҫͳһ���������ͷ���Դ)
    ((CriticalSection*)m_hCriticalSection)->Lock();
    this->m_clientSocketContexts[pConnection] = pSocketContext;
    ((CriticalSection*)m_hCriticalSection)->UnLock();

    // �ص�֪ͨ�ϲ�
    OnAccepted(pConnection, (CONST BYTE*)pIOContext->wsaBuf.buf, dwBytes);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // 5. ʹ�����֮�󣬰� Listen Socket ���Ǹ� IoContext ���ã�Ȼ��׼��Ͷ���µ� AcceptEx
    pIOContext->ResetBuffer();

    return this->_PostAccept(pIOContext);
}

BOOL IOCP::_PostAccept(HANDLE hHandle)
{
    PER_IO_CONTEXT_PTR pAcceptIOContext = static_cast<PER_IO_CONTEXT_PTR>(hHandle);
    PER_SOCKET_CONTEXT_PTR pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(this->m_hListenContext);

    //::ASSERT(INVALID_SOCKET != pSocketContext->socket);

    // ׼������
    pAcceptIOContext->opType = OverlappedOpType::Accept;
    DWORD dwBytes = 0;
    WSABUF *pWSABuffer = &pAcceptIOContext->wsaBuf;
    OVERLAPPED *pOverlapped = &pAcceptIOContext->overlapped;

    // Ϊ�Ժ�������Ŀͻ�����׼���� Socket
    pAcceptIOContext->opSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == pAcceptIOContext->opSocket)
    {
        // ��������Accept��Socketʧ��
        OnError(NULL, WSAGetLastError());
        return FALSE;
    }

    // Ͷ�� AcceptEx
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
            // Ͷ�� AcceptEx ����ʧ��
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
    // �Ȱ���һ�ε�������ʾ���֣�Ȼ�������״̬��������һ�� Recv ����
    OnRecved((IConnectionBase*)pSocketContext->pConnection, (CONST BYTE*)pIOContext->wsaBuf.buf, dwBytes);
    // Ȼ��ʼͶ����һ��WSARecv����
    return _PostRecv(pIOContext);
}

BOOL IOCP::_PostRecv(HANDLE hHandle)
{
    PER_IO_CONTEXT_PTR pIOContext = static_cast<PER_IO_CONTEXT_PTR>(hHandle);
    PER_SOCKET_CONTEXT_PTR pListenSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(this->m_hListenContext);
    // ��ʼ������
    DWORD dwFlags = 0;
    DWORD dwBytes = 0;
    WSABUF *pWSABuffer = &pIOContext->wsaBuf;
    OVERLAPPED *pOverlapped = &pIOContext->overlapped;

    pIOContext->ResetBuffer();
    pIOContext->opType = OverlappedOpType::Recv;

    // ��ʼ����ɺ�Ͷ�� WSARecv ����
    INT32 nBytesRecv = ::WSARecv(pIOContext->opSocket, pWSABuffer, 1, &dwBytes, &dwFlags, pOverlapped, NULL);

    // �������ֵ���󣬲��Ҵ���Ĵ��벢���� Pending �Ļ����Ǿ�˵������ص�����ʧ����
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
    // ��ӳ������Ƴ�
    this->m_clientSocketContexts.erase(pConnection);
    // �ͷ���������Ϣ
    delete pSocketContext;
    ((CriticalSection*)this->m_hCriticalSection)->UnLock();
    // ֪ͨ�ϲ�
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

    //������ɶ˿�
    ::CreateIoCompletionPort((HANDLE)hSocketContext->socket, m_hCompletionPort, (DWORD)hSocketContext, 0);
    if (SOCKET_ERROR == ::listen(hSocketContext->socket, SOMAXCONN))
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
    PER_SOCKET_CONTEXT_PTR hListenContext = static_cast<PER_SOCKET_CONTEXT_PTR>(m_hListenContext);
    for (int i = 0; i < MAX_POST_ACCEPT; i++)
    {
        // �½�һ��IO_CONTEXT
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
    // ��ӳ������ҵ����Ӷ����Ӧ����������Ϣ
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
        // �Ƚ�������ָ���ó���
        pSocketContext = static_cast<PER_SOCKET_CONTEXT_PTR>(found->second);
        // �ٴ�ӳ������Ƴ���ǰ���Ӷ������Ϣ
        this->m_clientSocketContexts.erase(pConnection);
        // �ͷ���������Ϣ
        delete pSocketContext;
    }
    ((CriticalSection*)this->m_hCriticalSection)->UnLock();
    // ֪ͨ�ϲ�
    OnRemoved(pConnection);
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