#pragma once
#include <map>
#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#else
#include <pthread.h>
#define GetCurrentThreadId pthread_self
#endif

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
        static ThreadLocalStorage<T>* tls = nullptr;
        if (nullptr == tls)
        {
            tls = new ThreadLocalStorage<T>();
        }
        return *tls;
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
