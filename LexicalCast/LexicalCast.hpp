#pragma once
#include <type_traits>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <cstring>
#define TRUE_STRING  "true"
#define FALSE_STRING "false"
namespace std
{
	template <typename To, typename From>
	struct _Converter
	{
	};

	//to numeric
	template <typename From>
	struct _Converter<int, From>
	{
		static int convert(const From& from)
		{
			return std::atoi(from);
		}
	};

	template <typename From>
	struct _Converter<long, From>
	{
		static long convert(const From& from)
		{
			return std::atol(from);
		}
	};

	template <typename From>
	struct _Converter<long long, From>
	{
		static long long convert(const From& from)
		{
			return std::atoll(from);
		}
	};

	template <typename From>
	struct _Converter<double, From>
	{
		static double convert(const From& from)
		{
			return std::atof(from);
		}
	};

	template <typename From>
	struct _Converter<float, From>
	{
		static float convert(const From& from)
		{
			return (float)std::atof(from);
		}
	};

	//to bool
	template <typename From>
	struct _Converter<bool, From>
	{
		static typename std::enable_if<std::is_integral<From>::value, bool>::type convert(From from)
		{
			return !!from;
		}
	};

	static bool _check_bool(const char* from, const size_t len, const char* s)
	{
		for (size_t i = 0; i < len; i++)
		{
			if (from[i] != s[i])
			{
				return false;
			}
		}

		return true;
	}

	static bool _convert(const char* from)
	{
		const unsigned int len = strlen(from);
		if (len != 4 && len != 5)
			throw std::invalid_argument("argument is invalid");

		bool r = true;
		if (len == 4)
		{
			r = _check_bool(from, len, TRUE_STRING);

			if (r)
				return true;
		}
		else
		{
			r = _check_bool(from, len, FALSE_STRING);

			if (r)
				return false;
		}

		throw std::invalid_argument("argument is invalid");
	}

	template <>
	struct _Converter<bool, string>
	{
		static bool convert(const string& from)
		{
			return std::_convert(from.c_str());
		}
	};

	template <>
	struct _Converter<bool, const char*>
	{
		static bool convert(const char* from)
		{
			return std::_convert(from);
		}
	};

	template <>
	struct _Converter<bool, char*>
	{
		static bool convert(char* from)
		{
			return std::_convert(from);
		}
	};

	template <unsigned N>
	struct _Converter<bool, const char[N]>
	{
		static bool convert(const char(&from)[N])
		{
			return std::_convert(from);
		}
	};

	template <unsigned N>
	struct _Converter<bool, char[N]>
	{
		static bool convert(const char(&from)[N])
		{
			return std::_convert(from);
		}
	};

	//to string
	template <typename From>
	struct _Converter<string, From>
	{
		static string convert(const From& from)
		{
			return std::to_string(from);
		}
	};
}

template <typename To, typename From>
typename std::enable_if<!std::is_same<To, From>::value, To>::type lexical_cast(const From& from)
{
	return std::_Converter<To, From>::convert(from);
}

template <typename To, typename From>
typename std::enable_if<std::is_same<To, From>::value, To>::type lexical_cast(const From& from)
{
	return from;
}

#undef TRUE_STRING
#undef FALSE_STRING