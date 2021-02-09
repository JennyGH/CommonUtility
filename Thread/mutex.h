#ifndef _MUTEX_H_
#define _MUTEX_H_
#if ENBALED_CPP_11
#    include <mutex>
#    define scoped_lock unique_lock
#    pragma message("Use c++11 mutex.")
#else
namespace std
{
    // Recursive mutex.
    class mutex
    {
    public:
        mutex();
        ~mutex();
        void  lock();
        void  unlock();
        void* internal_object();

    private:
        void* imp_;
    };

    template <class Mtx>
    class scoped_lock
    {
    public:
        scoped_lock(Mtx& mtx)
            : mtx_(&mtx)
        {
            mtx_->lock();
        }
        ~scoped_lock()
        {
            mtx_->unlock();
        }
        Mtx* mutex()
        {
            return this->mtx_;
        }

    private:
        Mtx* mtx_;
    };
} // namespace std
#endif // ENBALED_CPP_11
#endif // !_MUTEX_H_