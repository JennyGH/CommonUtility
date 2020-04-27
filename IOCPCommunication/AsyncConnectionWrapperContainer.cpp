#include "StdAfx.h"
#include "AsyncConnectionWrapperContainer.h"

#define LOCK \
m_lock.Lock();\
common::raii::scope_member_function_noparam<CriticalSection, void> scope_lock(&m_lock, &CriticalSection::UnLock)

AsyncConnectionWrapperContainer::AsyncConnectionWrapperContainer()
{
}

AsyncConnectionWrapperContainer::~AsyncConnectionWrapperContainer()
{
}

UINT32 AsyncConnectionWrapperContainer::GetCountOfConnections() const
{
	return m_connections.size();
}

AsyncConnectionWrapperPtr AsyncConnectionWrapperContainer::GetConnection()
{
	LOCK;
	AsyncConnectionWrapperPtr connection = m_connections.front();
	m_connections.pop();
	return connection;
}

void AsyncConnectionWrapperContainer::RecycleConnection(const AsyncConnectionWrapperPtr& connection)
{
	LOCK;
	m_connections.push(connection);
}
