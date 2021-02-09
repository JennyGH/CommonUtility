#pragma once
#include <functional>

#if defined(WIN32) || defined(_WIN32) || defined(_WINDOWS)
#ifndef WINAPI
#define WINAPI __stdcall
#endif // !WINAPI
#else //Not windows
#ifndef WINAPI
#define WINAPI
#endif
#endif // If windows

namespace common
{
    namespace raii
    {

        class _scope_raii_base
        {
            _scope_raii_base(const _scope_raii_base&) {}
            _scope_raii_base& operator= (const _scope_raii_base&) {}
        public:
            _scope_raii_base() {}
            ~_scope_raii_base() {}
        };


        template<typename _return, typename... _args>
        class scope_function_ex :
            public _scope_raii_base
        {
            typedef _return(*_func)(_args...);
        public:
            scope_function_ex(_func function, _args... params) :
                m_function(std::bind(function, params...)) {}
            ~scope_function_ex() { m_function(); }
        private:
            std::function<_return()> m_function;
        };

        template<typename _return, typename... _args>
        using scope_function = scope_function_ex<_return, _args...>;

        template<typename _return, typename... _args>
        class scope_stdcall_function_ex :
            public _scope_raii_base
        {
            typedef _return(WINAPI * _func)(_args...);
        public:
            scope_stdcall_function_ex(_func function, _args... params) :
                m_function(std::bind(function, params...)) {}
            ~scope_stdcall_function_ex() { m_function(); }
        private:
            std::function<_return()> m_function;
        };

        template<typename _return, typename... _args>
        using scope_stdcall_function = scope_stdcall_function_ex<_return, _args...>;

        template<class T, typename _return_, typename _param_>
        class scope_member_function : public _scope_raii_base
        {
            typedef _return_(T::*_func_)(_param_);
        public:
            scope_member_function(T* object, _func_ function, _param_ param) :m_object(object), m_function(function), m_param(param) {}
            ~scope_member_function()
            {
                if (m_object != NULL && m_function != NULL)
                {
                    (m_object->*m_function)(m_param);
                }
            }
        private:
            T * m_object;
            _func_    m_function;
            _param_    m_param;
        };

        template<class T, typename _return_>
        class scope_member_function_noparam : public _scope_raii_base
        {
            typedef _return_(T::*_func_)();
        public:
            scope_member_function_noparam(T* object, _func_ function) :m_object(object), m_function(function) {}
            ~scope_member_function_noparam()
            {
                if (m_object != NULL && m_function != NULL)
                {
                    (m_object->*m_function)();
                }
            }
        private:
            T * m_object;
            _func_    m_function;
        };

        //释放单对象内存
        template<typename T>
        static void release_object(T** mem)
        {
            if (*mem != NULL)
            {
                delete (*mem);
                (*mem) = NULL;
            }
        }
        //释放数组内存
        template<typename T>
        static void release_array(T** mem)
        {
            if (*mem != NULL)
            {
                delete[](*mem);
                (*mem) = NULL;
            }
        }

    }
}