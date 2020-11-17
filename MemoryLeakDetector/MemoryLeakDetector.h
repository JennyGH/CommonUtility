#pragma once
#include <string>

#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#ifndef GetCurrentThreadId
#define GetCurrentThreadId                pthread_self
#endif // !GetCurrentThreadId
#ifndef CRITICAL_SECTION
#define CRITICAL_SECTION                  pthread_mutex_t
#endif // !CRITICAL_SECTION
#ifndef InitializeCriticalSection
#define InitializeCriticalSection(pMutex) pthread_mutex_init(pMutex, NULL)
#endif // !InitializeCriticalSection
#ifndef DeleteCriticalSection
#define DeleteCriticalSection(pMutex)     pthread_mutex_destroy(pMutex)
#endif // !DeleteCriticalSection
#ifndef EnterCriticalSection
#define EnterCriticalSection(pMutex)      pthread_mutex_lock(pMutex)
#endif // !EnterCriticalSection
#ifndef LeaveCriticalSection
#define LeaveCriticalSection(pMutex)      pthread_mutex_unlock(pMutex)
#endif // !LeaveCriticalSection
#endif // defined(WIN32)||defined(_WIN32)

class MemoryLeakDetector
{
    struct MemoryBlock
    {
        void*         address;
        std::size_t   size;
        std::size_t   tid;
        char*         filename;
        int           line;
        MemoryBlock*  prev;
        MemoryBlock*  next;
    };

    MemoryBlock* FindBlock(void* address);

    MemoryLeakDetector(std::size_t id = 0);
public:
    ~MemoryLeakDetector();

    static MemoryLeakDetector& GetGlobalDetector();

    void InsertBlock(void* address, std::size_t size, const char* file = NULL, int line = 0);

    void RemoveBlock(void* address);

    std::size_t GetCurrentThreadMemoryLeak();

    std::size_t GetLeakMemotySize() const;

private:
    CRITICAL_SECTION		m_mutex;
    std::size_t		        m_nProcessId;
    std::size_t		        m_nLeakMemorySize;
    std::size_t		        m_nLeakBlockCount;
    MemoryBlock*	        m_pHead;
    MemoryBlock*	        m_pTail;
};

void* operator new  (std::size_t size, const char* file, int line);
void* operator new[](std::size_t size, const char* file, int line);
void* operator new  (std::size_t size, void* where, const char* file, int line);
void* operator new[](std::size_t size, void* where, const char* file, int line);
void  operator delete  (void* ptr);
void  operator delete[](void* ptr);

#ifdef DEBUG_NEW
#undef DEBUG_NEW
#endif // DEBUG_NEW

//#define DEBUG_NEW(_where) new(_where, __FILE__, __LINE__)
//#define new(_where) DEBUG_NEW(_where)

#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW
