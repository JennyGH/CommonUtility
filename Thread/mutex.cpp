#include "mutex.h"
#if !ENBALED_CPP_11
#    if WIN32
#        include <Windows.h>
typedef CRITICAL_SECTION mutex_core_type;
#    else
#        include <unistd.h>
#        include <pthread.h>
typedef pthread_mutex_t mutex_core_type;

static inline void InitializeCriticalSection(pthread_mutex_t* mtx)
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

#    define RECURSIVE_MUTEX_IMP static_cast<mutex_core_type*>(imp_)

std::mutex::mutex()
    : imp_(new mutex_core_type())
{
    ::InitializeCriticalSection(RECURSIVE_MUTEX_IMP);
}

std::mutex::~mutex()
{
    ::DeleteCriticalSection(RECURSIVE_MUTEX_IMP);
}

void std::mutex::lock()
{
    ::EnterCriticalSection(RECURSIVE_MUTEX_IMP);
}

void std::mutex::unlock()
{
    ::LeaveCriticalSection(RECURSIVE_MUTEX_IMP);
}

void* std::mutex::internal_object()
{
    return this->imp_;
}

#endif // !ENBALED_CPP_11