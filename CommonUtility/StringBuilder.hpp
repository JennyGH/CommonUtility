#pragma once
#include <list>
#include <string>
namespace common
{
	namespace text
	{
		template <typename chr>
		class StringBuilder {
			typedef std::basic_string<chr> string_t;
			typedef std::list<string_t> container_t;
			typedef typename string_t::size_type size_type;
		private:
			void _append(const string_t &src) {
				m_Data.push_back(src);
				m_totalSize += src.size();
			}
			// No copy constructor, no assignement.
			StringBuilder(const StringBuilder &) {};
			StringBuilder & operator = (const StringBuilder &) { return *this; };
		private:
			container_t m_Data;
			size_type m_totalSize;

		public:
			StringBuilder(const string_t &src) :m_totalSize(src.size()) {
				if (!src.empty())
				{
					m_Data.push_back(src);
				}
			}
			StringBuilder() :m_totalSize(0) { }

			StringBuilder & append(const string_t &src) {
				_append(src);
				return *this;
			}

			// This one lets you add any STL container to the string builder. 
			template<class inputIterator>
			StringBuilder & add(const inputIterator &first, const inputIterator &afterLast) {
				for (inputIterator f = first; f != afterLast; ++f) {
					_append(*f);
				}
				return *this;
			}

			StringBuilder & appendLine(const string_t &src) {
				static const chr lineFeed[] = { 13, 10, 0 };
				m_Data.push_back(src + lineFeed);
				m_totalSize += 2;
				m_totalSize += src.size();
				return *this;
			}
			StringBuilder & appendLine() {
				static const chr lineFeed[] = { 13, 10, 0 };
				m_Data.push_back(lineFeed);
				m_totalSize += 2;
				return *this;
			}

			operator string_t() const {
				string_t result;
				result.reserve(m_totalSize + 1);
				typename StringBuilder::container_t::const_iterator iter = m_Data.begin();
				typename StringBuilder::container_t::const_iterator end = m_Data.end();
				for (; iter != end; ++iter)
				{
					result += *iter;
				}
				return result;
			}

		};

	}
}