#pragma once
#include <map>
#include "LinuxMapping.h"

template<typename T>
class ThreadLocalStorage
{
public:
	typedef unsigned long ThreadID;
	typedef T ThreadData;
private:
	ThreadLocalStorage();
public:
	static ThreadLocalStorage& Get();
	~ThreadLocalStorage();

	ThreadData Store(const ThreadData& data);
	ThreadData GetData() const;
	std::size_t Size() const;

private:
	std::map<ThreadID, ThreadData> m_storage;
};

template<typename T>
inline ThreadLocalStorage<T> & ThreadLocalStorage<T>::Get()
{
	static ThreadLocalStorage<T> tls;
	return tls;
}

template<typename T>
inline ThreadLocalStorage<T>::~ThreadLocalStorage()
{
}

template<typename T>
inline ThreadLocalStorage<T>::ThreadData ThreadLocalStorage<T>::Store(const ThreadData & data)
{
	return (m_storage[::GetCurrentThreadId()] = data);
}

template<typename T>
inline ThreadLocalStorage<T>::ThreadData ThreadLocalStorage<T>::GetData() const
{
	return m_storage[::GetCurrentThreadId()];
}

template<typename T>
inline std::size_t ThreadLocalStorage<T>::Size() const
{
	return m_storage.size();
}
