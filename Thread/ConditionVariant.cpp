#include "ConditionVariant.h"

ConditionVariant::ConditionVariant()
{
#if defined(WIN32) || defined(_WIN32)
    ::InitializeConditionVariable(&m_cond);
#else
    pthread_cond_init(&m_cond, NULL);
#endif // defined(WIN32)||defined(_WIN32)
}

ConditionVariant::~ConditionVariant()
{
#if defined(WIN32) || defined(_WIN32)
#else
    pthread_cond_destroy(&m_cond);
#endif // defined(WIN32)||defined(_WIN32)
}

void ConditionVariant::Wait(CRITICAL_SECTION * criticalSection, PredicateCallback callback, void* context)
{
    if (NULL == callback)
    {
        return;
    }
    while (!callback(context))
    {
#if defined(WIN32) || defined(_WIN32)
        ::SleepConditionVariableCS(&m_cond, criticalSection, INFINITE);
#else
        pthread_cond_wait(&m_cond, criticalSection);
#endif // defined(WIN32)||defined(_WIN32)
    }
}

void ConditionVariant::Notify()
{
#if defined(WIN32) || defined(_WIN32)
    ::WakeConditionVariable(&m_cond);
#else
    pthread_cond_signal(&m_cond);
#endif // defined(WIN32)||defined(_WIN32)
}
