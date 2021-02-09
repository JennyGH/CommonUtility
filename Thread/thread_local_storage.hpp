#ifndef _THREAD_LOCAL_STORAGE_H_
#define _THREAD_LOCAL_STORAGE_H_
#if ENBALED_CPP_11
#    pragma message("Use C++11 thread local storage: `thread_local`.")
#    define __thread_local(type) thread_local type
#else
#    pragma message("Use C++98 thread local storage, ONLY support for value of POD type!")
#    if defined(WIN32) || defined(_WIN32)
#        include <Windows.h>
template <typename T>
class thread_local_storage
{
    // Noncopyable!!!
    thread_local_storage(const thread_local_storage&);
    thread_local_storage& operator=(const thread_local_storage&);

public:
    typedef T                 value_type;
    typedef value_type&       value_ref_type;
    typedef const value_type& value_const_ref_type;

private:
    static value_type* get_value(DWORD index)
    {
        return static_cast<value_type*>(::TlsGetValue(index));
    }

public:
    thread_local_storage()
        : index_(::TlsAlloc())
    {
    }

    thread_local_storage(value_const_ref_type value)
        : index_(::TlsAlloc())
    {
        this->operator=(value);
    }

    ~thread_local_storage()
    {
        if (TLS_OUT_OF_INDEXES != index_)
        {
            value_type* pValue = get_value(index_);
            if (NULL != pValue)
            {
                delete pValue;
                pValue = NULL;
            }
            BOOL bSuccess = ::TlsFree(index_);
            index_        = TLS_OUT_OF_INDEXES;
        }
    }

    operator value_type() const
    {
        if (TLS_OUT_OF_INDEXES == index_)
        {
            return value_type();
        }
        value_type* pValue = get_value(index_);
        if (NULL == pValue)
        {
            return value_type();
        }
        return *pValue;
    }

    value_type operator=(value_const_ref_type value)
    {
        if (TLS_OUT_OF_INDEXES == index_)
        {
            return value_type();
        }
        value_type* pValue = get_value(index_);
        if (NULL == pValue)
        {
            pValue = new value_type();
            ::TlsSetValue(index_, pValue);
        }
        *pValue = value;
        return value;
    }

private:
    DWORD index_;
};
#        define __thread_local(type) thread_local_storage<type>
#    else
#        include <pthread.h>
#        define __thread_local(type) __thread type
#    endif // defined(WIN32) || defined(_WIN32)
#endif     // ENBALED_CPP_11
#endif     // !_THREAD_LOCAL_STORAGE_H_