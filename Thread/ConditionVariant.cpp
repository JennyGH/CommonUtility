#include "ConditionVariant.h"

ConditionVariant::ConditionVariant()
{
#if defined(WIN32) || defined(_WIN32)
    ::InitializeConditionVariable(&m_cond);
#else
    pthread_cond_init(&m_event, NULL);
#endif // defined(WIN32)||defined(_WIN32)
}

ConditionVariant::~ConditionVariant()
{
#if defined(WIN32) || defined(_WIN32)
#else
    pthread_cond_destroy(&m_event);
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
        pthread_cond_wait(&m_event, &m_mutex);
#endif // defined(WIN32)||defined(_WIN32)
    }
}

void ConditionVariant::Notify()
{
#if defined(WIN32) || defined(_WIN32)
    ::WakeConditionVariable(&m_cond);
#else
    pthread_cond_signal(&m_event);
#endif // defined(WIN32)||defined(_WIN32)
}
