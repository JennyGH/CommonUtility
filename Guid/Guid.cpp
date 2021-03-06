#include "Guid.h"
#ifdef WIN32
#    include <objbase.h>
#else
/*   ========== INSTALATION OF UUID ==========
 *   On Ubuntu: sudo apt install uuid-dev
 *   On CentOS: sudo yum install libuuid-devel
 *   =========================================
 */
#    include <uuid/uuid.h>
#    include <stdio.h>
#    include <stdlib.h>
#    include <string.h>
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

#endif // WIN32

Guid::Guid()
    : m_a(0x00000000UL)
    , m_b(0x0000UL)
    , m_c(0x0000UL)
    , m_d(0x00)
    , m_e(0x00)
    , m_f(0x00)
    , m_g(0x00)
    , m_h(0x00)
    , m_i(0x00)
    , m_j(0x00)
    , m_k(0x00)
{
#ifdef WIN32
    GUID guid = {0};
    if (CoCreateGuid(&guid) == S_OK)
    {
        m_a = guid.Data1;
        m_b = guid.Data2;
        m_c = guid.Data3;
        m_d = guid.Data4[0];
        m_e = guid.Data4[1];
        m_f = guid.Data4[2];
        m_g = guid.Data4[3];
        m_h = guid.Data4[4];
        m_i = guid.Data4[5];
        m_j = guid.Data4[6];
        m_k = guid.Data4[7];
    }
#else
    uuid_t uu;
    uuid_generate(uu);
    if (sizeof(uu) < 16)
    {
        // Fail to generate uuid.
        return;
    }
    m_a = (m_a << 8) | uu[0];
    m_a = (m_a << 8) | uu[1];
    m_a = (m_a << 8) | uu[2];
    m_a = (m_a << 8) | uu[3];
    m_b = (m_b << 8) | uu[4];
    m_b = (m_b << 8) | uu[5];
    m_c = (m_c << 8) | uu[6];
    m_c = (m_c << 8) | uu[7];

    m_d = uu[8];
    m_e = uu[9];
    m_f = uu[10];
    m_g = uu[11];
    m_h = uu[12];
    m_i = uu[13];
    m_j = uu[14];
    m_k = uu[15];
#endif // WIN32
}

Guid::Guid(const std::string& guid)
{
    unsigned long a = 0;
    unsigned int  b = 0;
    unsigned int  c = 0;
    unsigned int  d = 0;
    unsigned int  e = 0;
    unsigned int  f = 0;
    unsigned int  g = 0;
    unsigned int  h = 0;
    unsigned int  i = 0;
    unsigned int  j = 0;
    unsigned int  k = 0;
    sscanf_s(
        guid.c_str(),
        "%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        &a,
        &b,
        &b,
        &d,
        &e,
        &f,
        &g,
        &h,
        &i,
        &j,
        &k);

    m_a = a;
    m_b = b;
    m_c = c;
    m_d = d;
    m_e = e;
    m_f = f;
    m_g = g;
    m_h = h;
    m_i = i;
    m_j = j;
    m_k = k;
}

Guid::Guid(const Guid& that)
    : m_a(that.m_a)
    , m_b(that.m_b)
    , m_c(that.m_c)
    , m_d(that.m_d)
    , m_e(that.m_e)
    , m_f(that.m_f)
    , m_g(that.m_g)
    , m_h(that.m_h)
    , m_i(that.m_i)
    , m_j(that.m_j)
    , m_k(that.m_k)
{
}

Guid::~Guid() {}

Guid& Guid::operator=(const Guid& that)
{
    if (&that != this)
    {
        m_a = that.m_a;
        m_b = that.m_b;
        m_c = that.m_c;
        m_d = that.m_d;
        m_e = that.m_e;
        m_f = that.m_f;
        m_g = that.m_g;
        m_h = that.m_h;
        m_i = that.m_i;
        m_j = that.m_j;
        m_k = that.m_k;
    }
    return *this;
}

Guid::operator std::string() const
{
    std::string res;

    char a[16] = {0};
    char b[16] = {0};
    char c[16] = {0};
    char d[16] = {0};
    char e[16] = {0};
    char f[16] = {0};
    char g[16] = {0};
    char h[16] = {0};
    char i[16] = {0};
    char j[16] = {0};
    char k[16] = {0};

    sprintf_s(a, sizeof(a), "%08lX", m_a);
    sprintf_s(b, sizeof(b), "%04X", m_b);
    sprintf_s(c, sizeof(c), "%04X", m_c);
    sprintf_s(d, sizeof(d), "%02X", m_d);
    sprintf_s(e, sizeof(e), "%02X", m_e);
    sprintf_s(f, sizeof(f), "%02X", m_f);
    sprintf_s(g, sizeof(g), "%02X", m_g);
    sprintf_s(h, sizeof(h), "%02X", m_h);
    sprintf_s(i, sizeof(i), "%02X", m_i);
    sprintf_s(j, sizeof(j), "%02X", m_j);
    sprintf_s(k, sizeof(k), "%02X", m_k);

    res.append(a, 8).append(1, '-');
    res.append(b, 4).append(1, '-');
    res.append(c, 4).append(1, '-');
    res.append(d, 2);
    res.append(e, 2).append(1, '-');
    res.append(f, 2);
    res.append(g, 2);
    res.append(h, 2);
    res.append(i, 2);
    res.append(j, 2);
    res.append(k, 2);

    return res;
}

std::string Guid::GetBytes() const
{
#define APPEND_BYTES(res, member, size)                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        unsigned char member[size] = {0};                                                                              \
        memcpy_s(member, size, &m_##member, size);                                                                     \
        res.append(member, member + size);                                                                             \
    } while (0)
    std::string res;

    APPEND_BYTES(res, a, 4);
    APPEND_BYTES(res, b, 2);
    APPEND_BYTES(res, c, 2);
    APPEND_BYTES(res, d, 1);
    APPEND_BYTES(res, e, 1);
    APPEND_BYTES(res, f, 1);
    APPEND_BYTES(res, g, 1);
    APPEND_BYTES(res, h, 1);
    APPEND_BYTES(res, i, 1);
    APPEND_BYTES(res, j, 1);
    APPEND_BYTES(res, k, 1);

    return res;
}