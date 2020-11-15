#pragma once
#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#define CRITICAL_SECTION                  pthread_mutex_t
#define InitializeCriticalSection(pMutex) pthread_mutex_init(pMutex, NULL)
#define DeleteCriticalSection(pMutex)     pthread_mutex_destroy(pMutex)
#define EnterCriticalSection(pMutex)      pthread_mutex_lock(pMutex)
#define LeaveCriticalSection(pMutex)      pthread_mutex_unlock(pMutex)
#endif // defined(WIN32)||defined(_WIN32)

typedef bool(*ConditionCallback)(void* context);

class ConditionVariant
{
public:
	ConditionVariant(CRITICAL_SECTION & mutex);
	~ConditionVariant();

	void Wait(ConditionCallback callback, void* context);

	void Notify();

private:
	CRITICAL_SECTION& m_mutex;
#if defined(WIN32) || defined(_WIN32)
	HANDLE            m_event;
#else
	pthread_cond_t    m_event;
#endif // defined(WIN32)||defined(_WIN32)

};