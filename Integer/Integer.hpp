#pragma once
#include <string>

#if !defined(WIN32) && !defined(_WIN32)

#ifndef sprintf_s
#define sprintf_s(buffer, bufferSize, format, ...) sprintf(buffer, format, ##__VA_ARGS__)
#endif // !sprintf_s

#ifndef sscanf_s
#define sscanf_s(bfufer, format, ...) sscanf(buffer, format, ##__VA_ARGS__)
#endif // !sscanf_s

#endif // !defined(WIN32) && !defined(_WIN32)


template<typename T>
struct _Default
{
	static std::string ToString(T val) { return std::string(); }
	static T FromString(const std::string& src) { return T(); }
};

#define RETURN_FORMATED_STRING(bufferSize, format, val) \
do{\
	char _buffer[bufferSize] = { 0 };\
	sprintf_s(_buffer, format, val);\
	return _buffer;\
} while (0)


#define DECLARE_DEFAULT(type, format) \
template<>struct _Default<type>{\
static std::string ToString(type val) { RETURN_FORMATED_STRING(64, format, val); }\
static type FromString(const std::string& src) { type val = type(); sscanf_s(src.c_str(), format, &val); return val;  }\
}

DECLARE_DEFAULT(short, "%d");
DECLARE_DEFAULT(int, "%d");
DECLARE_DEFAULT(long, "%ld");
DECLARE_DEFAULT(long long, "%lld");

template<typename T> class Integer;
typedef Integer<short> Short;
typedef Integer<int> Int32;
typedef Integer<long> Long;
typedef Integer<long long> Int64;

template<typename T>
class Integer
{
	static T FromBytes(const unsigned char src[]);
	static T FromString(const char src[], const char fmt[]);
public:
	static const T Max = (1LL << (sizeof(T) * 8 - 1)) - 1;
	static const T Min = (1LL << (sizeof(T) * 8 - 1)) + 1;
public:
	Integer();
	Integer(T val);
	Integer(const char src[], const char fmt[] = NULL);
	Integer(const unsigned char src[]);
	~Integer();
	operator T() const;
	operator std::string() const;
	std::string ToString(const std::string& fmt = "") const;
private:
	T m_val;
};

template<typename T>
inline T Integer<T>::FromBytes(const unsigned char src[])
{
	return T();
}

template<typename T>
inline T Integer<T>::FromString(const char src[], const char fmt[])
{
	T val = 0;

	if (NULL == fmt)
	{
		return _Default<T>::FromString(src);
	}
	else
	{
		sscanf_s(src, fmt, &val);
	}
	return val;
}

template<typename T>
inline Integer<T>::Integer() : m_val(0) {}

template<typename T>
inline Integer<T>::Integer(T val) : m_val(val) {}

template<typename T>
inline Integer<T>::Integer(const char src[], const char fmt[]) : m_val(0)
{
	if (src != NULL)
	{
		m_val = FromString(src, fmt);
	}
}

template<typename T>
inline Integer<T>::Integer(const unsigned char src[]) : m_val(0)
{
	if (src != NULL)
	{
		m_val = FromBytes(src);
	}
}

template<typename T>
inline Integer<T>::~Integer() {}

template<typename T>
inline Integer<T>::operator T() const
{
	return m_val;
}

template<typename T>
inline Integer<T>::operator std::string() const
{
	return ToString();
}

template<typename T>
inline std::string Integer<T>::ToString(const std::string& fmt) const
{
	if (fmt.empty())
	{
		return _Default<T>::ToString(m_val);
	}

	RETURN_FORMATED_STRING(64, fmt.c_str(), m_val);
}
