#pragma once
#include <list>
#include <string>
namespace common
{
	namespace text
	{
		template <typename chr>
		class string_builder {
			typedef std::basic_string<chr> string_t;
			typedef std::list<string_t> container_t;
			typedef typename string_t::size_type size_type;
		private:
			void _append(const string_t &src)
			{
				m_data.push_back(src);
				m_totalSize += src.size();
			}
			// No copy constructor, no assignement.
			string_builder(const string_builder &) {};
			string_builder & operator = (const string_builder &) { return *this; };
		private:
			container_t m_data;
			size_type m_totalSize;

		public:
			string_builder(const string_t &src) :
				m_totalSize(src.size())
			{
				if (!src.empty())
				{
					m_data.push_back(src);
				}
			}
			string_builder() :m_totalSize(0) { }

			string_builder & append(const string_t &src)
			{
				_append(src);
				return *this;
			}

			// This one lets you add any STL container to the string builder. 
			template<class inputIterator>
			string_builder & add(const inputIterator &first, const inputIterator &afterLast)
			{
				for (inputIterator f = first; f != afterLast; ++f)
				{
					_append(*f);
				}
				return *this;
			}

			string_builder & appendLine(const string_t &src)
			{
#ifdef _WIN32
				static const chr lineFeed[] = { 13, 10, 0 }; // \r\n
#elif __APPLE__
				static const chr lineFeed[] = { 13, 0 }; // \r
#else
				static const chr lineFeed[] = { 10, 0 }; // \n
#endif
				m_data.push_back(src + lineFeed);
				m_totalSize += 2;
				m_totalSize += src.size();
				return *this;
			}
			string_builder & appendLine()
			{
#ifdef _WIN32
				static const chr lineFeed[] = { 13, 10, 0 }; // \r\n
#elif __APPLE__
				static const chr lineFeed[] = { 13, 0 }; // \r
#else
				static const chr lineFeed[] = { 10, 0 }; // \n
#endif
				m_data.push_back(lineFeed);
				m_totalSize += 2;
				return *this;
			}

			operator string_t() const
			{
				string_t result;
				result.reserve(m_totalSize + 1);
				typename string_builder::container_t::const_iterator iter = m_data.begin();
				typename string_builder::container_t::const_iterator end = m_data.end();
				for (; iter != end; ++iter)
				{
					result += *iter;
				}
				return result;
			}

		};

	}
}