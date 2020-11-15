#include "stdafx.h"
#include "ConditionVariant.h"

ConditionVariant::ConditionVariant(CRITICAL_SECTION & mutex)
	: m_mutex(mutex)
{
#if defined(WIN32) || defined(_WIN32)
	m_event = CreateEvent(NULL, FALSE, TRUE, NULL);
#else
	pthread_cond_init(&m_event, NULL);
#endif // defined(WIN32)||defined(_WIN32)
}

ConditionVariant::~ConditionVariant()
{
#if defined(WIN32) || defined(_WIN32)
	CloseHandle(m_event);
#else
	pthread_cond_destroy(&m_event);
#endif // defined(WIN32)||defined(_WIN32)
}

void ConditionVariant::Wait(ConditionCallback callback, void* context)
{
	if (NULL == callback)
	{
		return;
	}
	while (callback(context))
	{
#if defined(WIN32) || defined(_WIN32)
		WaitForSingleObject(m_event, INFINITE);
#else
		pthread_cond_wait(&m_event, &m_mutex);
#endif // defined(WIN32)||defined(_WIN32)
	}
}

void ConditionVariant::Notify()
{
#if defined(WIN32) || defined(_WIN32)
	SetEvent(m_event);
#else
	pthread_cond_signal(&m_event);
#endif // defined(WIN32)||defined(_WIN32)
}
