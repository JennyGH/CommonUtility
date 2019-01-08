#pragma once
#include "Globals.h"
#include <string.h>

namespace common
{
	namespace raii
	{
		template<typename T>
		class DynamicBuffer
		{
		public:
			typedef typename long size_type;
		public:
			DynamicBuffer() :m_data(null), m_size(0) {}
			DynamicBuffer(DynamicBuffer::size_type size) :m_data(null), m_size(0)
			{
				allocate(size);
			}
			DynamicBuffer(const T data[], DynamicBuffer::size_type size) :m_data(null), m_size(0)
			{
				if (size > 0)
				{
					m_size = size;
					m_data = new T[m_size]();
					memcpy_s(m_data, m_size, data, size);
				}
			}

			DynamicBuffer(const DynamicBuffer& that) :m_data(null), m_size(0)
			{
				if (that.m_size > 0)
				{
					allocate(that.m_size);
					memcpy_s(m_data, m_size, that.m_data, that.m_size);
				}
			}

			~DynamicBuffer()
			{
				release();
			}

			DynamicBuffer& operator= (const DynamicBuffer& that)
			{
				return copy(that.m_data, that.m_size);
			}

			DynamicBuffer& copy(const T data[], DynamicBuffer::size_type size)
			{
				if (size > 0)
				{
					resize(size);
					memcpy_s(m_data, m_size, data, size);
				}
				return *this;
			}

			T* allocate(DynamicBuffer::size_type size)
			{
				if (size > 0)
				{
					m_size = size;
					m_data = new T[m_size]();
					memset(m_data, 0, m_size);
				}
				return m_data;
			}

			T* resize(DynamicBuffer::size_type size)
			{
				release();
				return allocate(size);
			}

			DynamicBuffer::size_type size() const
			{
				return m_size;
			}

			operator T*()
			{
				return m_data;
			}

			operator const T*() const
			{
				return m_data;
			}

		private:
			void release()
			{
				if (null != m_data)
				{
					delete[] m_data;
					m_data = null;
				}
				m_size = 0;
			}

		private:
			T* m_data;
			DynamicBuffer::size_type m_size;
		};
	}
}