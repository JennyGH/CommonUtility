#include "event.h"
#ifdef WIN32
#    include <Windows.h>
typedef HANDLE event_handle;
#else
#    include <errno.h>
#    include <sys/time.h>
#    include <pthread.h>
typedef struct
{
    bool            state;
    bool            manual_reset;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} event_t, *event_handle;
#endif

#define CORE ((event_handle)(m_handle))

event::event(bool bManualReset, bool bInitState)
#ifdef WIN32
    : m_handle(::CreateEvent(NULL, bManualReset, bInitState, NULL))
#else
    : m_handle(new event_t())
#endif
    , m_code(0)
{
#ifndef WIN32
    CORE->state        = bInitState;
    CORE->manual_reset = bManualReset;
    if (pthread_mutex_init(&CORE->mutex, NULL))
    {
        delete CORE;
        m_handle = NULL;
        return;
    }
    if (pthread_cond_init(&CORE->cond, NULL))
    {
        pthread_mutex_destroy(&CORE->mutex);
        delete CORE;
        m_handle = NULL;
        return;
    }
#endif
}

event::~event()
{
    destroy();
}

int event::wait()
{
#ifdef WIN32
    DWORD ret = ::WaitForSingleObject(CORE, INFINITE);
    if (ret != WAIT_OBJECT_0)
    {
        goto fail;
    }
#else
    if (pthread_mutex_lock(&CORE->mutex))
    {
        goto fail;
    }
    while (!CORE->state)
    {
        if (pthread_cond_wait(&CORE->cond, &CORE->mutex))
        {
            pthread_mutex_unlock(&CORE->mutex);
            goto fail;
        }
    }
    if (!CORE->manual_reset)
    {
        CORE->state = false;
    }
    if (pthread_mutex_unlock(&CORE->mutex))
    {
        goto fail;
    }
#endif
    return m_code;

fail:
#if WIN32
    return ::GetLastError();
#else
    return errno;
#endif // WIN32
}

int event::wait(unsigned long milliseconds)
{
#ifdef WIN32
    DWORD ret = ::WaitForSingleObject(CORE, milliseconds);
    if (ret != WAIT_OBJECT_0) // FAILED!
    {
        switch (ret)
        {
            case WAIT_TIMEOUT:
                return event ::timeout;
        }
        goto fail;
    }
#else  // !WIN32
    int             rc      = 0;
    struct timespec abstime = {0};
    struct timeval  tv      = {0};
    gettimeofday(&tv, NULL);
    abstime.tv_sec  = tv.tv_sec + milliseconds / 1000;
    abstime.tv_nsec = tv.tv_usec * 1000 + (milliseconds % 1000) * 1000000;
    if (abstime.tv_nsec >= 1000000000)
    {
        abstime.tv_nsec -= 1000000000;
        abstime.tv_sec++;
    }
    if (pthread_mutex_lock(&CORE->mutex) != 0)
    {
        goto fail;
    }
    while (!CORE->state)
    {
        if ((rc = pthread_cond_timedwait(&CORE->cond, &CORE->mutex, &abstime)))
        {
            if (rc == ETIMEDOUT)
            {
                break;
            }
            pthread_mutex_unlock(&CORE->mutex);
            goto fail;
        }
    }
    if (rc == 0 && !CORE->manual_reset)
    {
        CORE->state = false;
    }
    if (pthread_mutex_unlock(&CORE->mutex) != 0)
    {
        goto fail;
    }
    if (rc == ETIMEDOUT)
    {
        return event::timeout;
    }
#endif // WIN32

    return m_code;

fail:
    return event::fail;
}

void event::notify(int code)
{
    m_code = code;
#ifdef WIN32
    ::SetEvent(CORE);
#else
    if (pthread_mutex_lock(&CORE->mutex) != 0)
    {
        return;
    }

    CORE->state = true;

    if (CORE->manual_reset)
    {
        if (pthread_cond_broadcast(&CORE->cond))
        {
            return;
        }
    }
    else
    {
        if (pthread_cond_signal(&CORE->cond))
        {
            return;
        }
    }

    if (pthread_mutex_unlock(&CORE->mutex) != 0)
    {
        return;
    }

    return;
#endif
}

void event::reset()
{
#ifdef WIN32
    ::ResetEvent(CORE);
#else
    if (pthread_mutex_lock(&CORE->mutex) != 0)
    {
        return;
    }

    CORE->state = false;

    if (pthread_mutex_unlock(&CORE->mutex) != 0)
    {
        return;
    }
#endif
}

void event::destroy()
{
#ifdef WIN32
    ::CloseHandle(CORE);
#else
    pthread_cond_destroy(&CORE->cond);
    pthread_mutex_destroy(&CORE->mutex);
    delete CORE;
#endif
    this->m_handle = NULL;
}
