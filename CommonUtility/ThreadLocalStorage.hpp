#pragma once
#include <map>
#include <Windows.h>

template<typename T>
class ThreadLocalStorage
{
public:
	typedef unsigned long ThreadID;
	typedef T ThreadData;
private:
	ThreadLocalStorage() {}
public:
	static ThreadLocalStorage& Instance()
	{
		static ThreadLocalStorage<T> tls;
		return tls;
	}

	~ThreadLocalStorage()
	{
	}

	ThreadData Store(const ThreadData& data)
	{
		return (m_storage[::GetCurrentThreadId()] = data);
	}

	ThreadData Get()
	{
		return m_storage[::GetCurrentThreadId()];
	}

	ThreadData& Reference()
	{
		return m_storage[::GetCurrentThreadId()];
	}

	const ThreadData& ConstReference()
	{
		return m_storage[::GetCurrentThreadId()];
	}

	operator ThreadData()
	{
		return Get();
	}

	std::size_t Size() const
	{
		return m_storage.size();
	}

private:
	std::map<ThreadID, ThreadData> m_storage;
};
