#pragma once
#include <vector>
#include <string>
namespace std
{
    template <typename _Chr = char>
    class basic_string_ex : public std::basic_string<_Chr>
    {
    public:
        typedef std::basic_string<_Chr>      string_t;
        typedef std::vector<basic_string_ex> splited_t;
        typedef void (*foreach_func)(_Chr);
        typedef void (*foreach_func_ref)(_Chr&);
        typedef void (*foreach_func_const_ref)(const _Chr&);

    public:
        basic_string_ex() {};
        basic_string_ex(const string_t& src)
            : string_t(src) {};
        basic_string_ex(const _Chr src[])
            : string_t(src) {};
        basic_string_ex(const _Chr src[], typename string_t::size_type len)
            : string_t(src, len) {};
        basic_string_ex(const unsigned char src[], typename string_t::size_type len)
            : string_t(src, src + len) {};
        basic_string_ex(const std::istreambuf_iterator<_Chr>& begin, const std::istreambuf_iterator<_Chr>& end)
            : string_t(begin, end) {};
        ~basic_string_ex() {};

        basic_string_ex& remove(_Chr target)
        {
            typename string_t::size_type _pos = this->find(target);
            while (_pos != string_t::npos)
            {
                this->erase(_pos, 1);
                _pos = this->find(target);
            }
            return *this;
        }

        basic_string_ex& remove(const string_t& target)
        {
            typename string_t::size_type _pos = this->find(target);
            while (_pos != string_t::npos)
            {
                this->erase(_pos, target.length());
                _pos = this->find(target);
            }
            return *this;
        }

        basic_string_ex& tolower()
        {
            typename string_t::iterator iter = this->begin();
            typename string_t::iterator end  = this->end();
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

        basic_string_ex& toupper()
        {
            typename string_t::iterator iter = this->begin();
            typename string_t::iterator end  = this->end();
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

        splited_t split(const string_t& separator) const
        {
            splited_t                    output;
            typename string_t::size_type current = 0;
            typename string_t::size_type end     = -1;
            typename string_t::size_type len     = separator.length();
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

            if (this->find_last_of(separator) == this->length() - separator.length())
            {
                output.push_back("");
            }

            return output;
        }

        basic_string_ex& replace(const basic_string_ex& from, const basic_string_ex& to)
        {
            basic_string_ex&             me        = *this;
            typename string_t::size_type total     = 0;
            splited_t                    split_res = split(from);
            for (typename splited_t::const_iterator iter = split_res.begin(); iter != split_res.end(); ++iter)
            {
                total += iter->length();
            }
            total += ((split_res.size() - 1) * to.length());
            this->clear();
            this->reserve(total + 1);
            for (typename splited_t::const_iterator iter = split_res.begin(); iter != split_res.end(); ++iter)
            {
                if (iter != split_res.begin())
                {
                    me += to;
                }
                me += *iter;
            }
            return *this;
        }

        basic_string_ex& foreach (foreach_func func)
        {
            if (func != NULL)
            {
                typename string_t::const_iterator iter = this->begin();
                typename string_t::const_iterator end  = this->end();
                for (; iter != end; ++iter)
                {
                    func(*iter);
                }
            }
            return *this;
        }
        basic_string_ex& foreach (foreach_func_ref func)
        {
            if (func != NULL)
            {
                typename string_t::iterator iter = this->begin();
                typename string_t::iterator end  = this->end();
                for (; iter != end; ++iter)
                {
                    func(*iter);
                }
            }
            return *this;
        }
        basic_string_ex& foreach (foreach_func_const_ref func)
        {
            if (func != NULL)
            {
                typename string_t::const_iterator iter = this->begin();
                typename string_t::const_iterator end  = this->end();
                for (; iter != end; ++iter)
                {
                    func(*iter);
                }
            }
            return *this;
        }
    };
    typedef basic_string_ex<char>    string_ex;
    typedef basic_string_ex<wchar_t> wstring_ex;
} // namespace std
