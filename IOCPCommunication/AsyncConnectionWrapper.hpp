#pragma once
#include "IConnectionBase.h"
#include <memory>
// AsyncConnectionWrapper的智能指针
class AsyncConnectionWrapper;
using AsyncConnectionWrapperPtr = std::shared_ptr<AsyncConnectionWrapper>;
using ConnectionMap = std::map<const IConnectionBase*, AsyncConnectionWrapperPtr>;
/*
 * IConnection连接对象的异步包装
 */
class AsyncConnectionWrapper
{
public:
	AsyncConnectionWrapper(IConnectionBase* pConnection, UINT32 af, UINT32 protocol);
	~AsyncConnectionWrapper();

	//About socket.
	SOCKET Socket() const;
	IConnectionBase* Connection() const;
	void Close(LPFN_DISCONNECTEX lpfnDisconntEx);
	bool IsClosed() const;

	//About object.
	static AsyncConnectionWrapperPtr Find(const IConnectionBase* connection);
	static void Remove(const IConnectionBase* connection);

private:
	SOCKET				m_socket;
	bool				m_isConnectting;
	CriticalSection		m_lock;
	IConnectionBase*	m_pConnectionBase;

private:
	static CriticalSection	g_lock;
	static ConnectionMap	g_connections;
};
