#include "condition_variable.h"
#if !ENBALED_CPP_11
#    if WIN32
#        include <Windows.h>
#        if _WIN32_WINNT < 0x0600
#            error "condition_variable is not supported for windows xp."
#        endif // _WIN32_WINNT < 0x0600
typedef CONDITION_VARIABLE condition_variable_core_type;
typedef CRITICAL_SECTION   mutex_core_type;
#    else
#        include <unistd.h>
#        include <pthread.h>
typedef pthread_cond_t  condition_variable_core_type;
typedef pthread_mutex_t mutex_core_type;
static inline void      InitializeCriticalSection(pthread_mutex_t* mtx)
{
    ::pthread_mutexattr_t attr;
    ::pthread_mutexattr_init(&attr);
    ::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    ::pthread_mutex_init(mtx, &attr);
}
static inline void DeleteCriticalSection(pthread_mutex_t* mtx)
{
    ::pthread_mutex_destroy(mtx);
}
static inline void EnterCriticalSection(pthread_mutex_t* mtx)
{
    ::pthread_mutex_lock(mtx);
}
static inline void LeaveCriticalSection(pthread_mutex_t* mtx)
{
    ::pthread_mutex_unlock(mtx);
}
#    endif // WIN32

#    define CONDITION_VARIABLE_IMP  static_cast<condition_variable_core_type*>(imp_)
#    define INTERNAL_LOCK_OBJ(lock) static_cast<mutex_core_type*>(lock.mutex()->internal_object())

std::condition_variable::condition_variable()
    : imp_(new condition_variable_core_type())
{
#    if WIN32
    ::InitializeConditionVariable(CONDITION_VARIABLE_IMP);
#    else
    ::pthread_cond_init(CONDITION_VARIABLE_IMP, NULL);
#    endif // WIN32
}

std::condition_variable::~condition_variable()
{
#    if WIN32
    // do nothing.
#    else
    ::pthread_cond_destroy(CONDITION_VARIABLE_IMP);
#    endif // WIN32
    if (NULL != CONDITION_VARIABLE_IMP)
    {
        delete CONDITION_VARIABLE_IMP;
        imp_ = NULL;
    }
}

void std::condition_variable::wait(scoped_lock<mutex>& lock)
{
#    if WIN32
    ::SleepConditionVariableCS(CONDITION_VARIABLE_IMP, INTERNAL_LOCK_OBJ(lock), INFINITE);
#    else
    ::pthread_cond_wait(CONDITION_VARIABLE_IMP, INTERNAL_LOCK_OBJ(lock));
#    endif // WIN32
}

void std::condition_variable::notify_one()
{
#    if WIN32
    ::WakeConditionVariable(CONDITION_VARIABLE_IMP);
#    else
    ::pthread_cond_signal(CONDITION_VARIABLE_IMP);
#    endif // WIN32
}

void std::condition_variable::notify_all()
{
#    if WIN32
    ::WakeAllConditionVariable(CONDITION_VARIABLE_IMP);
#    else
    ::pthread_cond_broadcast(CONDITION_VARIABLE_IMP);
#    endif // WIN32
}
#endif // ENBALED_CPP_11