#include "stdafx.h"
#include "AsyncBuffer.h"
#include "AsyncConnectionWrapper.h"

#define lock \
m_lock.Lock();\
common::raii::scope_member_function_noparam<CriticalSection, void> scope_lock(&m_lock, &CriticalSection::UnLock)
#define GLOBAL_LOCK \
g_lock.Lock();\
common::raii::scope_member_function_noparam<CriticalSection, void> scope_lock(&g_lock, &CriticalSection::UnLock)

CriticalSection AsyncConnectionWrapper::g_lock;
ConnectionMap AsyncConnectionWrapper::g_connections;

AsyncConnectionWrapper::AsyncConnectionWrapper(IConnectionBase* pConnection, UINT32 af, UINT32 protocol) :
	m_socket(INVALID_SOCKET),
	m_pConnectionBase(pConnection),
	m_isConnectting(true)
{
	m_socket = WSASocket(af, SOCK_STREAM, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
	GLOBAL_LOCK;
	g_connections[m_pConnectionBase].reset(this);
}

AsyncConnectionWrapper::~AsyncConnectionWrapper()
{
	common::raii::release_object((IConnectionBase**)&m_pConnectionBase);
}

AsyncConnectionWrapperPtr AsyncConnectionWrapper::Find(const IConnectionBase* connection)
{
	GLOBAL_LOCK;

	if (connection == NULL)
	{
		return NULL;
	}

	ConnectionMap::iterator iter = g_connections.find(connection);
	if (iter != g_connections.end())
	{
		return iter->second;
	}

	return NULL;
}

void AsyncConnectionWrapper::Remove(const IConnectionBase * connection)
{
	AsyncConnectionWrapperPtr conn = AsyncConnectionWrapper::Find(connection);
	if (conn != NULL)
	{
		GLOBAL_LOCK;
		conn->m_isConnectting = false;
		g_connections.erase(connection);
	}
}

SOCKET AsyncConnectionWrapper::Socket() const
{
	return m_socket;
}

IConnectionBase * AsyncConnectionWrapper::Connection() const
{
	return m_pConnectionBase;
}

void AsyncConnectionWrapper::Close(LPFN_DISCONNECTEX lpfnDisconnectEx)
{
	if (lpfnDisconnectEx != NULL)
	{
		AsyncBuffer& buffer = AsyncBuffer::NewBuffer(AsyncConnectionWrapperPtr(this), AsyncOperation::Disconnect, 0);
		if (!lpfnDisconnectEx(m_socket, &(buffer.overlapped), TF_DISCONNECT | TF_REUSE_SOCKET, 0))
		{
		}
	}
	m_isConnectting = false;
}

bool AsyncConnectionWrapper::IsClosed() const
{
	return !m_isConnectting;
}
