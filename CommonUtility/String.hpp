#pragma once
#ifndef NULL
#define NULL 0
#endif // !NULL

#include <list>
#include <string>


namespace common
{
	namespace text
	{
		template<typename _Chr = char>
		class string : public std::basic_string< _Chr>
		{
			typedef std::basic_string< _Chr> string_t;
			typedef std::list<string> split_res_t;
		public:
			typedef void(*foreach_func)(_Chr);
			typedef void(*foreach_func_ref)(_Chr&);
			typedef void(*foreach_func_const_ref)(const _Chr&);
		public:
			string() {};
			string(const string_t& src) : string_t(src) {};
			string(const _Chr src[]) : string_t(src) {};
			string(const _Chr src[], typename string_t::size_type len) : string_t(src, len) {};
			string(const unsigned char src[], typename string_t::size_type len) : string_t(src, src + len) {};
			string(const std::istreambuf_iterator<_Chr>& begin, const std::istreambuf_iterator<_Chr>& end) : string_t(begin, end) {};
			~string() {};

#define __remove(target)\
do{\
	typename string_t::size_type _pos = this->find(target);\
	while (_pos != string_t::npos)\
	{\
		this->erase(this->begin() + _pos);\
		_pos = this->find(target);\
	}\
}while (0)

			string& remove(_Chr target)
			{
				__remove(target);
				return *this;
			}

			string& remove(const string_t& target)
			{
				__remove(target);
				return *this;
			}

			string& tolower()
			{
				typename string_t::iterator iter = this->begin();
				typename string_t::iterator end = this->end();
				for (; iter != end; ++iter)
				{
					_Chr c = *iter;
					if ((0x41 <= c && c <= 0x5a) || (0x61 <= c && c <= 0x7a))
					{
						*iter = ::tolower(*iter);
					}
				}
				return *this;
			}

			string& toupper()
			{
				typename string_t::iterator iter = this->begin();
				typename string_t::iterator end = this->end();
				for (; iter != end; ++iter)
				{
					_Chr c = *iter;
					if ((0x41 <= c && c <= 0x5a) || (0x61 <= c && c <= 0x7a))
					{
						*iter = ::toupper(*iter);
					}
				}
				return *this;
			}

			string& split(const string_t& separator, std::list<string>& output)
			{
				output.clear();
				typename string_t::size_type current = 0;
				typename string_t::size_type end = -1;
				typename string_t::size_type len = separator.length();
				while (true)
				{
					end = this->find(separator, current);
					if (end == string_t::npos)
					{
						if (current < this->length())
						{
							output.push_back(this->substr(current));
						}
						break;
					}
					else
					{
						if (end - current > 0)
						{
							output.push_back(this->substr(current, end - current));
						}
						current = end + len;
					}
				}

				return *this;
			}

			string& replace(const string& from, const string& to)
			{
				string& me = *this;
				typename string_t::size_type total = 0;
				split_res_t split_res;
				split(from, split_res);
				for (typename split_res_t::const_iterator iter = split_res.begin(); iter != split_res.end(); ++iter)
				{
					total += iter->length();
				}
				total += ((split_res.size() - 1) * to.length());
				this->clear();
				this->reserve(total + 1);
				for (typename split_res_t::const_iterator iter = split_res.begin(); iter != split_res.end(); ++iter)
				{
					if (iter != split_res.begin())
					{
						me += to;
					}
					me += *iter;
				}
				return *this;
			}

			string& foreach(foreach_func func)
			{
				if (func != NULL)
				{
					typename string_t::const_iterator iter = this->begin();
					typename string_t::const_iterator end = this->end();
					for (; iter != end; ++iter)
					{
						func(*iter);
					}
				}
				return *this;
			}
			string& foreach(foreach_func_ref func)
			{
				if (func != NULL)
				{
					typename string_t::iterator iter = this->begin();
					typename string_t::iterator end = this->end();
					for (; iter != end; ++iter)
					{
						func(*iter);
					}
				}
				return *this;
			}
			string& foreach(foreach_func_const_ref func)
			{
				if (func != NULL)
				{
					typename string_t::const_iterator iter = this->begin();
					typename string_t::const_iterator end = this->end();
					for (; iter != end; ++iter)
					{
						func(*iter);
					}
				}
				return *this;
			}
		};
	}
}