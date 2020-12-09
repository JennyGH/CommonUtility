#pragma once
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

typedef bool(*PredicateCallback)(void* context);

class ConditionVariant
{
public:
    ConditionVariant();
    ~ConditionVariant();

    void Wait(CRITICAL_SECTION * criticalSection, PredicateCallback callback, void* context);

    void Notify();

private:
#if defined(WIN32) || defined(_WIN32)
    CONDITION_VARIABLE    m_cond;
#else
    pthread_cond_t        m_cond;
#endif // defined(WIN32)||defined(_WIN32)

};