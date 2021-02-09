#pragma once
#include <limits>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if !defined(WIN32) && !defined(_WIN32)
#    include <cassert>
#    include <cstdarg>

#    ifndef ASSERT
#        if _DEBUG
#            define ASSERT(prediction) assert((prediction))
#        else
#            define ASSERT(prediction) // no-op
#        endif
#    endif // !ASSERT

#    ifndef memcpy_s
void __memcpy_s(void* dest, std::size_t destSize, const void* src, std::size_t srcSize)
{
    ASSERT((dest != NULL) && (src != NULL));
    ASSERT((destSize >= srcSize));
    memcpy(dest, src, srcSize);
}
#        define memcpy_s __memcpy_s
#    endif // !memcpy_s

#    ifndef sscanf_s
int __sscanf_s(const char* buffer, const char* format, ...)
{
    ASSERT((buffer != NULL) && (format != NULL));
    va_list args;
    va_start(args, format);
    int sscanfedSize = vsscanf(buffer, format, args);
    va_end(args);
    return sscanfedSize;
}
#        define sscanf_s __sscanf_s
#    endif // !sscanf_s

#    ifndef sprintf_s
int __sprintf_s(char* dest, std::size_t destSize, const char* format, ...)
{
    ASSERT((dest != NULL) && (format != NULL));
    ASSERT((destSize > 0));
    va_list args;
    va_start(args, format);
    int sprintfedSize = vsnprintf(dest, destSize, format, args);
    ASSERT((destSize >= sprintfedSize));
    va_end(args);
    return sprintfedSize;
}
#        define sprintf_s __sprintf_s
#    endif // !sprintf_s

#endif // !defined(WIN32) && !defined(_WIN32)

template <typename T>
struct _Default
{
    static std::string ToString(T val)
    {
        return std::string();
    }
    static T FromString(const std::string& src)
    {
        return T();
    }
};

#define RETURN_FORMATED_STRING(bufferSize, format, val)                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        char _buffer[bufferSize] = {0};                                                                                \
        sprintf_s(_buffer, bufferSize, format, val);                                                                   \
        return _buffer;                                                                                                \
    } while (0)

#define DECLARE_DEFAULT(type, format)                                                                                  \
    template <>                                                                                                        \
    struct _Default<type>                                                                                              \
    {                                                                                                                  \
        static std::string ToString(type val)                                                                          \
        {                                                                                                              \
            RETURN_FORMATED_STRING(64, format, val);                                                                   \
        }                                                                                                              \
        static type FromString(const std::string& src)                                                                 \
        {                                                                                                              \
            type val = 0;                                                                                              \
            sscanf_s(src.c_str(), format, &val);                                                                       \
            return val;                                                                                                \
        }                                                                                                              \
    }

DECLARE_DEFAULT(short, "%hd");
DECLARE_DEFAULT(int, "%d");
DECLARE_DEFAULT(long, "%ld");
DECLARE_DEFAULT(long long, "%lld");

template <typename T>
class Integer;
typedef Integer<short>     Short;
typedef Integer<int>       Int32;
typedef Integer<long>      Long;
typedef Integer<long long> Int64;

template <typename T>
class Integer
{
    static T FromBytes(const unsigned char src[], unsigned int size);
    static T FromString(const char src[], const char fmt[]);

public:
    static const Integer<T> Max;
    static const Integer<T> Min;

public:
    Integer();
    Integer(T val);
    Integer(const char src[], const char fmt[] = NULL);
    Integer(const unsigned char src[], unsigned int size);
    ~Integer();
                operator T() const;
                operator std::string() const;
    std::string ToString(const std::string& fmt = "") const;

private:
    T m_val;
};
template <typename T>
const Integer<T> Integer<T>::Max = std::numeric_limits<T>::max(); //(1LL << (sizeof(T) * 8 - 1)) - 1;
template <typename T>
const Integer<T> Integer<T>::Min = std::numeric_limits<T>::min(); //(1LL << (sizeof(T) * 8 - 1)) + 1;

template <typename T>
inline T Integer<T>::FromBytes(const unsigned char src[], unsigned int size)
{
    T val = T();
    memcpy_s(&val, sizeof(val), src, size);
    return val;
}

template <typename T>
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

template <typename T>
inline Integer<T>::Integer()
    : m_val(0)
{
}

template <typename T>
inline Integer<T>::Integer(T val)
    : m_val(val)
{
}

template <typename T>
inline Integer<T>::Integer(const char src[], const char fmt[])
    : m_val(0)
{
    if (src != NULL)
    {
        m_val = FromString(src, fmt);
    }
}

template <typename T>
inline Integer<T>::Integer(const unsigned char src[], unsigned int size)
    : m_val(0)
{
    if (src != NULL)
    {
        m_val = FromBytes(src, size);
    }
}

template <typename T>
inline Integer<T>::~Integer()
{
}

template <typename T>
inline Integer<T>::operator T() const
{
    return m_val;
}

template <typename T>
inline Integer<T>::operator std::string() const
{
    return ToString();
}

template <typename T>
inline std::string Integer<T>::ToString(const std::string& fmt) const
{
    if (fmt.empty())
    {
        return _Default<T>::ToString(m_val);
    }

    RETURN_FORMATED_STRING(64, fmt.c_str(), m_val);
}
