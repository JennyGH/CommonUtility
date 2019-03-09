#pragma once

#ifndef null
#define null NULL
#endif // !null

namespace common
{
	namespace memory
	{
		template<class T>
		class smart_pointer
		{
			template <class T>
			class pointer
			{
			public:
				pointer() : m_nRefCount(0), m_pObject(null) {};
				pointer(T* ptr) : m_nRefCount(null == ptr ? 0 : 1), m_pObject(ptr) {};
				~pointer()
				{
					if (null != m_pObject)
					{
						delete m_pObject;
						m_pObject = null;
					}
				};

				int increase_ref()
				{
					if (null != m_pObject)
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
			smart_pointer() : m_pPtr(null) {}
			smart_pointer(T* ptr) : m_pPtr(null == ptr ? null : new pointer<T>(ptr)) {}
			smart_pointer(const smart_pointer& that) :m_pPtr(that.m_pPtr)
			{
				if (null != m_pPtr)
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
				if (null != m_pPtr)
				{
					//如果引用计数为0，销毁
					if (m_pPtr->decrease_ref() == 0)
					{
						delete m_pPtr;
						m_pPtr = null;
					}
				}

				//指向新地址
				m_pPtr = that.m_pPtr;

				if (null != m_pPtr)
				{
					//引用计数+1
					m_pPtr->increase_ref();
				}

				return *this;
			}
			~smart_pointer()
			{
				if (null != m_pPtr)
				{
					if (m_pPtr->decrease_ref() == 0)
					{
						delete m_pPtr;
						m_pPtr = null;
					}
				}
			}

			operator T& ()
			{
				return const_cast<const smart_pointer*>(this)->operator const T& ();
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

		private:
			pointer<T>* m_pPtr;
		};

	}
}


