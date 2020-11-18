#pragma once
#include <list>
#include <string>
#include "IConnectionBase.h"

struct IOCPSettings
{
    UINT32	af;
    UINT32	protocol;
    bool	isAcceptWithData;
    int32_t	nCountOfThreads;
    UINT32	nRecvBufferSize;
    UINT32	nSendBufferSize;
};
#define DEFAULT_SEND_BUFFER_SIZE	-1
#define DEFAULT_RECV_BUFFER_SIZE	1024
#define DEFAULT_IOCP_SETTINGS		{ AF_INET, 0, FALSE, -1, DEFAULT_RECV_BUFFER_SIZE, DEFAULT_SEND_BUFFER_SIZE }

class IOCP
{
private:
    static unsigned WINAPI _WorkerThread(HANDLE hHandle);
private:
    BOOL _DoAccept(HANDLE hIOContext, DWORD dwBytes);
    BOOL _PostAccept();
    BOOL _DoRecv(HANDLE hSocketContext, HANDLE hIOContext, DWORD dwBytes);
    BOOL _PostRecv(HANDLE hSocketContext);
    VOID _RemoveClientSocketContext(HANDLE hSocketContext);
public:
    IOCP(IConnectionFactory * pConnectionFactory,
        const IOCPSettings&   settings);

    virtual ~IOCP();

    void         Start(const void* addr, UINT32 sizeOfAddress);
    virtual void Send(IConnectionBase* pConnection, const BYTE data[], UINT32 size);
    virtual void Remove(IConnectionBase* pConnection);
    virtual void Shutdown();

protected:
    // ====================== Callbacks ======================
    virtual void OnAccepted(IConnectionBase*, const BYTE[], UINT64) {};
    virtual void OnRecved(IConnectionBase*, const BYTE[], UINT64) {};
    virtual void OnSent(IConnectionBase*, const BYTE[], UINT64) {};
    virtual void OnError(IConnectionBase*, INT32) {};
    virtual void OnRemoved(IConnectionBase*) {};
    virtual void OnClientLost(IConnectionBase*) {};
    // =======================================================

private:
    UINT32				                    m_nAF;
    //UINT_PTR			                    m_socket;
    HANDLE                                  m_hListenSocketContext;
    UINT32				                    m_nProtocol;
    UINT32				                    m_nRecvBufferSize;
    UINT32				                    m_nSendBufferSize;
    HANDLE				                    m_pAddress;
    HANDLE				                    m_hCompletionPort;
    HANDLE				                    m_hCriticalSection;
    INT32				                    m_nCountOfThreads;
    IConnectionFactory*                     m_pFactory;
    bool				                    m_isAcceptWithData;
    std::map<IConnectionBase*, HANDLE>      m_clientSocketContexts;
};

