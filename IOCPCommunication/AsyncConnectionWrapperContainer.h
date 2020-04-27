#pragma once
#include <queue>
#include "AsyncConnectionWrapper.h"
#include "CriticalSection.h"

typedef std::queue<AsyncConnectionWrapperPtr> ConnectionQueue;

/*
 * 连接对象管理容器
 * 用于管理连接对象，实现复用连接对象
 */
class AsyncConnectionWrapperContainer
{
public:
	AsyncConnectionWrapperContainer();
	~AsyncConnectionWrapperContainer();
	UINT32 GetCountOfConnections() const;
	AsyncConnectionWrapperPtr GetConnection();
	void RecycleConnection(const AsyncConnectionWrapperPtr& connection);
private:
	CriticalSection m_lock;
	ConnectionQueue m_connections;
};

