#include "MemoryLeakDetector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#define GetCurrentProcessId getpid
#define GetCurrentThreadId	pthread_self
#endif

#define THREAD_LOCAL FALSE

MemoryLeakDetector::MemoryBlock * MemoryLeakDetector::FindBlock(void * address)
{
    MemoryLeakDetector::MemoryBlock* pBlock = m_pHead;
    while (NULL != pBlock)
    {
        if (pBlock->address == address)
        {
            return pBlock;
        }
        pBlock = pBlock->next;
    }
    return NULL;
}

MemoryLeakDetector::MemoryLeakDetector(std::size_t id)
    : m_nProcessId(id == 0 ? ::GetCurrentProcessId() : id)
    , m_nLeakMemorySize(0)
    , m_nLeakBlockCount(0)
    , m_pHead(NULL)
    , m_pTail(NULL)
{
    InitializeCriticalSection(&m_mutex);
}

MemoryLeakDetector::~MemoryLeakDetector()
{
    if (this->m_nLeakMemorySize > 0)
    {
        printf(
#if THREAD_LOCAL
            "Memory leak in thread[%lld] !!! \r\n"
#else
            "Memory leak in process[%lld] !!! \r\n"
#endif // THREAD_LOCAL
            "\t Total leak size: %lld bytes, count of leak block: %lld.\r\n",
            (std::size_t)this->m_nProcessId,
            (std::size_t)this->m_nLeakMemorySize,
            (std::size_t)this->m_nLeakBlockCount);
    }
    else
    {
        printf(
#if THREAD_LOCAL
            "No memory leak in thread[%lld].\r\n",
#else
            "No memory leak in process[%lld].\r\n",
#endif // THREAD_LOCAL
            (std::size_t)this->m_nProcessId);
    }
    // 释放所有节点内存
    // 找到头节点
    MemoryLeakDetector::MemoryBlock* ptr = this->m_pHead;
    while (NULL != ptr)
    {
        // 保存下一个节点指针
        MemoryLeakDetector::MemoryBlock* pNext = ptr->next;
        // 打印内存块大小、线程号、代码文件路径、行号
        printf("\t Block size: %d bytes, tid: %lld, file: %s, line: %d\r\n", ptr->size, ptr->tid, ptr->filename, ptr->line);
        // 删除当前元素
        ::free(ptr);
        // 移动游标指针
        ptr = pNext;
    }
    DeleteCriticalSection(&m_mutex);
}

MemoryLeakDetector & MemoryLeakDetector::GetGlobalDetector()
{
#if THREAD_LOCAL
    thread_local MemoryLeakDetector g_memoryLeakDetector(::GetCurrentThreadId());
#else
    static MemoryLeakDetector g_memoryLeakDetector;
#endif // THREAD_LOCAL

    return g_memoryLeakDetector;
}

void MemoryLeakDetector::InsertBlock(void * address, std::size_t size, const char* file, int line)
{
    EnterCriticalSection(&m_mutex);
    {
        MemoryLeakDetector::MemoryBlock* pBlock = (MemoryLeakDetector::MemoryBlock*)::malloc(sizeof(MemoryLeakDetector::MemoryBlock));
        memset(pBlock, 0, sizeof(MemoryLeakDetector::MemoryBlock));
        pBlock->address = address;
        pBlock->size = size;
        pBlock->line = line;
        pBlock->tid = ::GetCurrentThreadId();
        if (NULL != file)
        {
            int len = strlen(file);
            pBlock->filename = (char*)::malloc(sizeof(char) * (len + 1));
            memcpy(pBlock->filename, file, len);
            pBlock->filename[len] = 0;
        }
        m_nLeakMemorySize += size;
        m_nLeakBlockCount++;
        if (NULL == this->m_pHead || NULL == this->m_pTail)
        {
            //首次运行
            this->m_pHead = this->m_pTail = pBlock;
        }
        else
        {
            // 找到最后一个元素
            MemoryLeakDetector::MemoryBlock* pLast = this->m_pTail;
            pBlock->prev = pLast;
            pLast->next = pBlock;
            this->m_pTail = pBlock;
        }
    }
    LeaveCriticalSection(&m_mutex);
}

void MemoryLeakDetector::RemoveBlock(void * address)
{
    EnterCriticalSection(&m_mutex);
    {
        MemoryLeakDetector::MemoryBlock* pBlock = this->FindBlock(address);
        if (NULL != pBlock)
        {
            std::size_t size = pBlock->size;
            MemoryLeakDetector::MemoryBlock* pPrev = pBlock->prev;
            MemoryLeakDetector::MemoryBlock* pNext = pBlock->next;

            if (NULL != pPrev)
            {
                pPrev->next = pNext;
            }
            if (NULL != pNext)
            {
                pNext->prev = pPrev;
            }

            if (this->m_pHead == pBlock)
            {
                this->m_pHead = pNext;
            }
            if (this->m_pTail == pBlock)
            {
                this->m_pTail = pPrev;
            }
            if (NULL != pBlock->filename)
            {
                ::free(pBlock->filename);
                pBlock->filename = NULL;
            }
            ::free(pBlock);
            m_nLeakMemorySize -= size;
            m_nLeakBlockCount--;
        }
    }
    LeaveCriticalSection(&m_mutex);
}

std::size_t MemoryLeakDetector::GetCurrentThreadMemoryLeak()
{
    EnterCriticalSection(&m_mutex);

    bool printedFirstLine = false;
    std::size_t leak = 0;
    unsigned long tid = ::GetCurrentThreadId();

    MemoryLeakDetector::MemoryBlock* ptr = this->m_pHead;

    while (NULL != ptr)
    {
        if (ptr->tid == tid)
        {
            leak += ptr->size;
            if (!printedFirstLine)
            {
                printf("Memory leak in thread[%lld] !!! \r\n", tid);
                printedFirstLine = true;
            }
            printf("\t Block size: %d bytes, file: %s, line: %d\r\n", ptr->size, ptr->filename, ptr->line);
        }
    }

    LeaveCriticalSection(&m_mutex);

    return leak;
}

std::size_t MemoryLeakDetector::GetLeakMemotySize() const
{
    return m_nLeakMemorySize;
}

#undef new

void * operator new(std::size_t size, const char * file, int line)
{
    void* ptr = ::malloc(size);
    if (NULL == ptr)
    {
        printf("Out of memory\n");
        return ptr;
    }
    MemoryLeakDetector::GetGlobalDetector().InsertBlock(ptr, size, file, line);
    return ptr;
}

void * operator new[](std::size_t size, const char * file, int line)
{
    return operator new(size, file, line);
}

void* operator new(std::size_t size, void* where, const char* file, int line)
{
    void* ptr = ::malloc(size);
    if (NULL == ptr)
    {
        printf("Out of memory\n");
        return ptr;
    }
    MemoryLeakDetector::GetGlobalDetector().InsertBlock(ptr, size, file, line);
    return ptr;
}

void* operator new[](std::size_t size, void* where, const char* file, int line)
{
    return operator new(size, where, file, line);
}

void operator delete(void* ptr)
{
    MemoryLeakDetector::GetGlobalDetector().RemoveBlock(ptr);
    ::free(ptr);
}

void operator delete[](void* ptr)
{
    operator delete(ptr);
}