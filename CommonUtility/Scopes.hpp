#pragma once
#include "Globals.h"

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

		template<typename _return_, typename _param_>
		class scope_function :
			public _scope_raii_base
		{
			typedef _return_(*_func_)(_param_);
		public:
			scope_function(_func_ function, _param_ param) :m_param(param), m_function(function) {}
			~scope_function()
			{
				if (m_function != null)
				{
					m_function(m_param);
				}
			}
		private:
			_param_	m_param;
			_func_	m_function;
		};

		template<typename _return_>
		class scope_function_noparam :
			public _scope_raii_base
		{
			typedef _return_(*_func_)();
		public:
			scope_function_noparam(_func_ function) :m_function(function) {}
			~scope_function_noparam()
			{
				if (m_function != null)
				{
					m_function();
				}
			}
		private:
			_func_	m_function;
		};

		template<typename _return_, typename _param_>
		class scope_stdcall_function :
			public _scope_raii_base
		{
			typedef _return_(WINAPI * _func_)(_param_);
		public:
			scope_stdcall_function(_func_ function, _param_ param) :m_param(param), m_function(function) {}
			~scope_stdcall_function()
			{
				if (m_function != null)
				{
					m_function(m_param);
				}
			}
		private:
			_param_	m_param;
			_func_	m_function;
		};

		template<typename _return_>
		class scope_stdcall_function_noparam :
			public _scope_raii_base
		{
			typedef _return_(WINAPI * _func_)();
		public:
			scope_stdcall_function_noparam(_func_ function) :m_function(function) {}
			~scope_stdcall_function_noparam()
			{
				if (m_function != null)
				{
					m_function();
				}
			}
		private:
			_func_	m_function;
		};

		template<class T, typename _return_, typename _param_>
		class scope_member_function : public _scope_raii_base
		{
			typedef _return_(T::*_func_)(_param_);
		public:
			scope_member_function(T* object, _func_ function, _param_ param) :m_object(object), m_function(function), m_param(param) {}
			~scope_member_function()
			{
				if (m_object != null && m_function != null)
				{
					(m_object->*m_function)(m_param);
				}
			}
		private:
			T * m_object;
			_func_	m_function;
			_param_	m_param;
		};

		template<class T, typename _return_>
		class scope_member_function_noparam : public _scope_raii_base
		{
			typedef _return_(T::*_func_)();
		public:
			scope_member_function_noparam(T* object, _func_ function) :m_object(object), m_function(function) {}
			~scope_member_function_noparam()
			{
				if (m_object != null && m_function != null)
				{
					(m_object->*m_function)();
				}
			}
		private:
			T * m_object;
			_func_	m_function;
		};

		//释放单对象内存
		template<typename T>
		static void release_object(T** mem)
		{
			if (*mem != null)
			{
				delete (*mem);
				(*mem) = null;
			}
		}
		//释放数组内存
		template<typename T>
		static void release_array(T** mem)
		{
			if (*mem != null)
			{
				delete[](*mem);
				(*mem) = null;
			}
		}

	}
}