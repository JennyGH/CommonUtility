#pragma once
#include <queue>
#include "AsyncConnectionWrapper.h"
#include "CriticalSection.h"

typedef std::queue<AsyncConnectionWrapperPtr> ConnectionQueue;

/*
 * ���Ӷ����������
 * ���ڹ������Ӷ���ʵ�ָ������Ӷ���
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

