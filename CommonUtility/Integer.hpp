#pragma once

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
	operator const char*() const;
	const char* ToString(const char fmt[] = NULL) const;
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
		if (typeid(T) == typeid(short))
		{
			sscanf_s(src, "%d", &val);
		}
		else if (typeid(T) == typeid(int))
		{
			sscanf_s(src, "%d", &val);
		}
		else if (typeid(T) == typeid(long))
		{
			sscanf_s(src, "%ld", &val);
		}
		else if (typeid(T) == typeid(long long))
		{
			sscanf_s(src, "%lld", &val);
		}
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
inline Integer<T>::operator const char*() const
{
	return ToString();
}

template<typename T>
inline const char * Integer<T>::ToString(const char fmt[]) const
{
	static char _buffer[64] = { 0 };

	if (NULL == fmt)
	{
		if (typeid(T) == typeid(short))
		{
			sprintf_s(_buffer, "%d", (m_val));
		}
		else if (typeid(T) == typeid(int))
		{
			sprintf_s(_buffer, "%d", (m_val));
		}
		else if (typeid(T) == typeid(long))
		{
			sprintf_s(_buffer, "%ld", (m_val));
		}
		else if (typeid(T) == typeid(long long))
		{
			sprintf_s(_buffer, "%lld", (m_val));
		}
	}
	else
	{
		sprintf_s(_buffer, fmt, (m_val));
	}

	return _buffer;
}
