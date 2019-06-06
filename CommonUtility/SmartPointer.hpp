#pragma once

#ifndef NULL
#define NULL 0
#endif // !NULL

namespace common
{
	namespace memory
	{
		template<class T>
		class smart_pointer
		{
			class pointer
			{
			public:
				pointer() : m_nRefCount(0), m_pObject(NULL) {};
				pointer(T* ptr) : m_nRefCount(NULL == ptr ? 0 : 1), m_pObject(ptr) {};
				~pointer()
				{
					if (NULL != m_pObject)
					{
						delete m_pObject;
						m_pObject = NULL;
					}
				};

				int increase_ref()
				{
					if (NULL != m_pObject)
					{
						m_nRefCount++;
					}
					return m_nRefCount;
				}
				int decrease_ref()
				{
					if (--m_nRefCount < 0)
					{
						m_nRefCount = 0;
					}
					return m_nRefCount;
				}
				int  get_ref_count() const
				{
					return m_nRefCount;
				}
				T* object()
				{
					return m_pObject;
				}

			private:
				int m_nRefCount;
				T* m_pObject;
			};
		public:
			smart_pointer() : m_pPtr(NULL) {}
			smart_pointer(T* ptr) : m_pPtr(NULL)
			{
				if (NULL != ptr)
				{
					m_pPtr = new pointer(ptr);
				}
			}
			smart_pointer(const smart_pointer& that) :m_pPtr(that.m_pPtr)
			{
				if (NULL != m_pPtr)
				{
					m_pPtr->increase_ref();
				}
			}
			smart_pointer& operator= (const smart_pointer& that)
			{
				if (&that == this)
				{
					return *this;
				}

				//指向的旧地址引用-1
				if (NULL != m_pPtr)
				{
					//如果引用计数为0，销毁
					if (m_pPtr->decrease_ref() == 0)
					{
						delete m_pPtr;
						m_pPtr = NULL;
					}
				}

				//指向新地址
				m_pPtr = that.m_pPtr;

				if (NULL != m_pPtr)
				{
					//引用计数+1
					m_pPtr->increase_ref();
				}

				return *this;
			}
			~smart_pointer()
			{
				if (NULL != m_pPtr)
				{
					if (m_pPtr->decrease_ref() == 0)
					{
						delete m_pPtr;
						m_pPtr = NULL;
					}
				}
			}

			operator T& ()
			{
				return *(m_pPtr->object());
			}

			operator const T& () const
			{
				return *(m_pPtr->object());
			}

			T* operator->()
			{
				return const_cast<const smart_pointer*>(this)->operator->();
			}

			T* operator->() const
			{
				return m_pPtr->object();
			}

			T& operator*()
			{
				return const_cast<const smart_pointer*>(this)->operator*();
			}

			T& operator*() const
			{
				return *(m_pPtr->object());
			}

			template<typename _Cast>
			_Cast* cast()
			{
				return static_cast<_Cast*>(m_pPtr->object());
			}

			bool operator== (const void* ptr) const
			{
				if (ptr == this)
				{
					return true;
				}
				if (NULL == m_pPtr && NULL == ptr)
				{
					return true;
				}
				if (NULL == m_pPtr && NULL != ptr)
				{
					return false;
				}
				return m_pPtr->object() == ptr;
			}

			bool operator!= (const void* ptr) const
			{
				if (ptr == this)
				{
					return false;
				}
				if (NULL == m_pPtr && NULL == ptr)
				{
					return false;
				}
				if (NULL == m_pPtr && NULL != ptr)
				{
					return true;
				}
				return m_pPtr->object() != ptr;
			}

		private:
			pointer* m_pPtr;
		};

	}
}


