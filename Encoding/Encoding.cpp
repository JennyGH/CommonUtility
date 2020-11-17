#include "Encoding.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if __cplusplus >= 201103L
#include <codecvt>
#endif // __cplusplus >= 201103L
#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#else
#include <iconv.h>
#define CP_ACP                    0           // default to ANSI code page
#define CP_UTF8                   65001       // UTF-8 translation

//代码转换:从一种编码转为另一种编码
int code_convert(
    const char *    from_charset,
    const char *    to_charset,
    const char *    inbuf,
    std::size_t     inlen,
    char *          outbuf,
    std::size_t     outlen)
{
    iconv_t cd = ::iconv_open(to_charset, from_charset);
    if (cd == 0)
    {
        return -1;
    }
    memset(outbuf, 0, outlen);
    if (::iconv(cd, (char**)&inbuf, &inlen, &outbuf, &outlen) == -1)
    {
        ::iconv_close(cd);
        return -1;
    }
    ::iconv_close(cd);
    return 0;
}
//UNICODE码转为GB2312码
int utf8_to_gbk(const char *inbuf, int inlen, char *outbuf, int outlen)
{
    return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}
//GB2312码转为UNICODE码
int gbk_to_utf8(const char *inbuf, int inlen, char *outbuf, int outlen)
{
    return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}

static int MultiByteToWideChar(
    int         codePage,
    int         flags,
    const char* str,
    int         strLen,
    wchar_t*    wstr,
    int         wstrLen)
{
    ::setlocale(LC_ALL, "");
    char* tmp = new char[strLen]();
    switch (codePage)
    {
    case CP_ACP:
        utf8_to_gbk(str, strLen, tmp, strLen);
        break;
    case CP_UTF8:
        gbk_to_utf8(str, strLen, tmp, strLen);
        break;
    }
    str = tmp;
    int rv = ::mbstowcs(wstr, str, (strLen) * 2);
    printf("mbstowcs return: %d \n", rv);
    delete[] tmp;
    return rv;
}
static int WideCharToMultiByte(
    int             codePage,
    int             flags,
    const wchar_t*  wstr,
    int             wstrLen,
    char*           str,
    int             strLen,
    const char*     defaultChar,
    bool*           useDefaultChar)
{

    ::setlocale(LC_ALL, "");
    int rv = ::wcstombs(str, wstr, (wstrLen) * sizeof(wchar_t));
    printf("wcstombs return: %d \n", rv);
    char* tmp = new char[strLen]();
    switch (codePage)
    {
    case CP_ACP:
        utf8_to_gbk(str, strLen, tmp, strLen);
        break;
    case CP_UTF8:
        gbk_to_utf8(str, strLen, tmp, strLen);
        break;
    }
    str = tmp;
    delete[] tmp;
    return rv;
}
#endif // !defined(WIN32) && !defined(_WIN32)


enum CodePage
{
    GBK = 0,
    UTF8
};

Encoder Encoding::GBK(GBK);
Encoder Encoding::UTF8(UTF8);
Encoder Encoding::Default(GBK);

#if __cplusplus >= 201103L
typedef std::codecvt_utf8<wchar_t>                         utf8_convertor_type;
typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> gbk_convertor_type;

static std::string wstring_to_utf8string(const std::wstring & str)
{
    static std::wstring_convert<utf8_convertor_type> convertor;
    return convertor.to_bytes(str);
}

static std::wstring utf8string_to_wstring(const std::string & str)
{
    static std::wstring_convert<utf8_convertor_type> convertor;
    return convertor.from_bytes(str);
}

static std::string wstring_to_string(const std::wstring & str, const std::string & locale = "chs")
{
    static std::wstring_convert<gbk_convertor_type> convertor(new gbk_convertor_type(locale));
    return convertor.to_bytes(str);
}

static std::wstring string_to_wstring(const std::string & str, const std::string & locale = "chs")
{
    static std::wstring_convert<gbk_convertor_type> convertor(new gbk_convertor_type(locale));
    return convertor.from_bytes(str);
}
#else
static std::string wstring_to_utf8string(const std::wstring & str)
{
    if (str.empty())
    {
        return "";
    }

    const wchar_t* lpSrc = str.c_str();
    std::size_t length = str.length();

    int utfSize = ::WideCharToMultiByte(CP_UTF8, 0, lpSrc, length, 0, 0, NULL, NULL);
    if (utfSize <= 0)
    {
        return "";
    }

    char * utfStr = new char[utfSize + 1];
    if (NULL == utfStr)
    {
        return "";
    }
    memset(utfStr, 0, utfSize + 1);

    int iRes = 0;
    iRes = ::WideCharToMultiByte(CP_UTF8, 0, lpSrc, length, utfStr, utfSize, NULL, NULL);
    if (iRes <= 0)
    {
        delete[] utfStr;
        utfStr = NULL;
        return "";
    }

    std::string res(utfStr, iRes);
    delete[] utfStr;
    return res;
}

static std::wstring utf8string_to_wstring(const std::string & str)
{
    if (str.empty())
    {
        return L"";
    }

    const char* lpSrc = str.c_str();
    std::size_t length = str.length();

    int wchSize = ::MultiByteToWideChar(CP_UTF8, 0, lpSrc, length, 0, 0);
    if (wchSize <= 0)
    {
        return L"";
    }

    wchar_t * wchStr = new wchar_t[wchSize + 1];
    if (NULL == wchStr)
    {
        return L"";
    }
    wmemset(wchStr, 0, wchSize + 1);

    int iRes = ::MultiByteToWideChar(CP_UTF8, 0, lpSrc, length, wchStr, wchSize);
    if (iRes <= 0)
    {
        delete[] wchStr;
        wchStr = NULL;
        return L"";
    }

    std::wstring res(wchStr, iRes);
    delete[] wchStr;
    return res;
}

static std::string wstring_to_string(const std::wstring & str, const std::string & locale = "chs")
{
    if (str.empty())
    {
        return "";
    }

    const wchar_t* lpSrc = str.c_str();
    std::size_t length = str.length();

    int chSize = ::WideCharToMultiByte(CP_ACP, 0, lpSrc, length, 0, 0, 0, 0);
    if (chSize <= 0)
    {
        return "";
    }

    char * chStr = new char[chSize + 1];
    if (NULL == chStr)
    {
        return "";
    }
    memset(chStr, 0, chSize + 1);

    int iRes = 0;
    iRes = ::WideCharToMultiByte(CP_ACP, 0, lpSrc, length, chStr, chSize, NULL, NULL);
    if (iRes <= 0)
    {
        delete[] chStr;
        chStr = NULL;
        return "";
    }

    std::string res(chStr, iRes);
    delete[] chStr;
    return res;
}

static std::wstring string_to_wstring(const std::string & str, const std::string & locale = "chs")
{
    if (str.empty())
    {
        return L"";
    }

    const char* lpSrc = str.c_str();
    std::size_t length = str.length();

    int wideSize = ::MultiByteToWideChar(CP_ACP, 0, lpSrc, length, 0, 0);
    if (0 == wideSize)
    {
        return L"";
    }

    wchar_t * wideCh = new wchar_t[wideSize + 1];
    if (NULL == wideCh)
    {
        return L"";
    }
    wmemset(wideCh, 0, wideSize + 1);

    int iRes = 0;
    iRes = ::MultiByteToWideChar(CP_ACP, 0, lpSrc, length, wideCh, wideSize);
    if (0 == iRes)
    {
        delete[] wideCh;
        wideCh = NULL;
        return L"";
    }

    std::wstring res(wideCh, iRes);
    delete[] wideCh;
    return res;
}
#endif // __cplusplus >= 201103L


Encoder::Encoder(int codePage) :
    m_codePage(codePage),
    CodePage(m_codePage)
{
}

Encoder::Encoder(const Encoder & that) :
    m_codePage(that.m_codePage),
    CodePage(m_codePage)
{
}

Encoder::~Encoder()
{
}

std::string Encoder::GetString(const std::string & ascii) const
{
    switch (m_codePage)
    {
    case GBK:
    {
        return wstring_to_string(utf8string_to_wstring(ascii));
    }
    case UTF8:
    {
        return wstring_to_utf8string(string_to_wstring(ascii));
    }
    default:
        break;
    }
    return "";
}

std::wstring Encoder::GetString(const std::wstring & unicode) const
{
    switch (m_codePage)
    {
    case GBK:
    {
        return utf8string_to_wstring(wstring_to_string(unicode));
    }
    case UTF8:
    {
        return string_to_wstring(wstring_to_utf8string(unicode));
    }
    default:
        break;
    }
    return L"";
}

Encoder& Encoder::operator= (const Encoder& that)
{
    if (&that != this)
    {
        m_codePage = that.m_codePage;
    }
    return *this;
}

Encoding::Encoding() {}

Encoding::~Encoding() {}
