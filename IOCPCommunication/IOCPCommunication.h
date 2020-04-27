#pragma once
#include <string>
#include "ErrorHandler.h"
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

class IOCPCommunication
{
	void PostAccept();
	void PostRecv(HANDLE hHandle);
	static unsigned WINAPI _WorkerThread(void* hHandle);
public:
	IOCPCommunication(const IOCPSettings& settings, IConnectionFactory *pConnectionFactory = NULL);

	virtual ~IOCPCommunication();

	void Startup(const void* addr, UINT32 sizeOfAddress);
	virtual void Send(const IConnectionBase* pConnection, const BYTE data[], UINT32 size);
	virtual void Remove(const IConnectionBase* pConnection);
	virtual void Shutdown();

protected:
	//Callback functions.
	virtual void OnAccepted(const IConnectionBase*, const BYTE[], UINT64) {};
	virtual void OnRecved(const IConnectionBase*, const BYTE[], UINT64) {};
	virtual void OnSent(const IConnectionBase*, const BYTE[], UINT64) {};
	virtual void OnError(const IConnectionBase*, UINT64) {};
	virtual void OnRemoved(const IConnectionBase*) {};
	virtual void OnClientLost(const IConnectionBase*) {};

private:
	UINT32				m_nAF;
	UINT_PTR			m_nSocket;
	UINT32				m_nProtocol;
	UINT32				m_nRecvBufferSize;
	UINT32				m_nSendBufferSize;
	HANDLE				m_pAddress;
	HANDLE				m_hCompletionPort;
	HANDLE				m_hCriticalSection;
	HANDLE				m_hConnectionContainer;
	INT32				m_nCountOfThreads;
	IConnectionFactory* m_pFactory;
	bool				m_isAcceptWithData;
};

