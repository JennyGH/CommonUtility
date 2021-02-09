#ifndef _CONDITION_VARIABLE_H_
#define _CONDITION_VARIABLE_H_
#if ENBALED_CPP_11
#    include <condition_variable>
#    ifndef scoped_lock
#        define scoped_lock unique_lock
#    endif // !scoped_lock
#    pragma message("Use c++11 condition_variable.")
#else
#    include "mutex.h"
namespace std
{
    class condition_variable
    {
    public:
        condition_variable();

        ~condition_variable();

        void wait(scoped_lock<mutex>& lock);

        template <typename PredicateFunction>
        void wait(scoped_lock<mutex>& lock, PredicateFunction predicate)
        {
            while (!predicate())
            {
                this->wait(lock);
            }
        }

        void notify_one();

        void notify_all();

    private:
        void* imp_;
    };
} // namespace std
#endif // ENBALED_CPP_11
#endif // !_CONDITION_VARIABLE_H_