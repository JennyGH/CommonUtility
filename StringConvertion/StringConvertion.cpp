#include "StringConvertion.h"

CStringConvertion::CStringConvertion(void)
{
}

CStringConvertion::~CStringConvertion(void)
{
    size_t count = m_utf8Char.size();
    for (size_t s = 0; s < count; ++s)
    {
        char * lpChar = m_utf8Char[s];
        if (lpChar)
            delete[] lpChar;
    }
    m_utf8Char.clear();

    count = m_wideChar.size();
    for (size_t n = 0; n < count; ++n)
    {
        wchar_t * lpWchar = m_wideChar[n];
        if (lpWchar)
            delete[] lpWchar;
    }

    m_wideChar.clear();

    count = m_multiChar.size();
    for (size_t i = 0; i < count; ++i)
    {
        char * lpChar = m_multiChar[i];
        if (lpChar)
            delete[] lpChar;
    }
    m_multiChar.clear();
}


char * CStringConvertion::WideCharToMultiChar(const wchar_t * lpSrc, int length, int* outLength)
{
    if (NULL == lpSrc || outLength == NULL || length <= 0)
        return NULL;

    int chSize = ::WideCharToMultiByte(CP_ACP, 0, lpSrc, length, 0, 0, 0, 0);
    if (chSize <= 0)
        return NULL;

    char * chStr = new char[chSize + 1];
    if (NULL == chStr)
        return NULL;
    memset(chStr, 0, chSize + 1);

    int iRes = 0;
    iRes = ::WideCharToMultiByte(CP_ACP, 0, lpSrc, length, chStr, chSize, NULL, NULL);
    if (iRes <= 0)
    {
        delete[] chStr;
        chStr = NULL;
        return NULL;
    }

    *outLength = chSize;
    m_multiChar.push_back(chStr);
    return chStr;

}


char * CStringConvertion::WideCharToUTF8String(const wchar_t * lpSrc, int length, int* outLength)
{
    if (NULL == lpSrc || outLength == NULL || length <= 0)
        return NULL;

    int utfSize = WideCharToMultiByte(CP_UTF8, 0, lpSrc, length, 0, 0, NULL, NULL);
    if (utfSize <= 0)
        return NULL;

    char * utfStr = new char[utfSize + 1];
    if (NULL == utfStr)
        return NULL;
    memset(utfStr, 0, utfSize + 1);

    int iRes = 0;
    iRes = WideCharToMultiByte(CP_UTF8, 0, lpSrc, length, utfStr, utfSize, NULL, NULL);
    if (iRes <= 0)
    {
        delete[] utfStr;
        utfStr = NULL;
        return NULL;
    }

    *outLength = iRes;
    m_utf8Char.push_back(utfStr);

    return  utfStr;
}


std::string CStringConvertion::WideCharToUTF8String(const std::wstring& wstr)
{
    int out = 0;
    char* str = WideCharToUTF8String(wstr.c_str(), wstr.length(), &out);
    return std::string(str, out);
}

std::string CStringConvertion::MultiCharToUTF8String(const std::string & str)
{
    int len = 0;
    char* res = MultiCharToUTF8String(str.c_str(), str.length(), &len);
    return std::string(res, len);
}

std::wstring CStringConvertion::UTF8StringToWideChar(const std::string & str)
{
    int len = 0;
    wchar_t* res = UTF8StringToWideChar(str.c_str(), str.length(), &len);
    return std::wstring(res, len);
}

wchar_t * CStringConvertion::MultiCharToWideChar(const char * lpSrc, int length, int* outLength)
{
    if (NULL == lpSrc || length <= 0 || outLength == NULL)
        return NULL;

    int wideSize = MultiByteToWideChar(CP_ACP, 0, lpSrc, length, 0, 0);
    if (0 == wideSize)
        return NULL;

    wchar_t * wideCh = new wchar_t[wideSize + 1];
    if (NULL == wideCh)
        return NULL;
    wmemset(wideCh, 0, wideSize + 1);

    int iRes = 0;
    iRes = MultiByteToWideChar(CP_ACP, 0, lpSrc, length, wideCh, wideSize);
    if (0 == iRes)
    {
        delete[] wideCh;
        wideCh = NULL;
        return NULL;
    }

    *outLength = iRes;
    m_wideChar.push_back(wideCh);
    return wideCh;
}

char * CStringConvertion::MultiCharToUTF8String(const char * lpSrc, int length, int* outLength)
{
    if (NULL == lpSrc || length <= 0 || outLength == NULL)
        return NULL;

    //First convert the ansi to unicode
    int len = 0;
    wchar_t * wchStr = MultiCharToWideChar(lpSrc, length, &len);
    if (NULL == wchStr)
        return NULL;

    //Then convert the unicode to utf8
    char * utfStr = WideCharToUTF8String(wchStr, len, &len);
    if (NULL == utfStr)
    {
        return NULL;
    }
    *outLength = len;
    return utfStr;
}


wchar_t * CStringConvertion::UTF8StringToWideChar(const char * lpSrc, int length, int* outLength)
{
    if (NULL == lpSrc || length <= 0 || outLength == NULL)
        return NULL;

    int wchSize = MultiByteToWideChar(CP_UTF8, 0, lpSrc, length, 0, 0);
    if (wchSize <= 0)
        return NULL;

    wchar_t * wchStr = new wchar_t[wchSize + 1];
    if (NULL == wchStr)
        return NULL;
    wmemset(wchStr, 0, wchSize + 1);

    int iRes = MultiByteToWideChar(CP_UTF8, 0, lpSrc, length, wchStr, wchSize);
    if (iRes <= 0)
    {
        delete[] wchStr;
        wchStr = NULL;
        return NULL;
    }
    *outLength = iRes;
    m_wideChar.push_back(wchStr);

    return wchStr;
}


char * CStringConvertion::UTF8StringToMultiChar(const char * lpSrc, int length, int* outLength)
{
    if (NULL == lpSrc || length <= 0 || outLength == NULL)
        return NULL;

    //First convert utf8 to wide char
    int len = 0;
    wchar_t * wchStr = UTF8StringToWideChar(lpSrc, length, &len);
    if (NULL == wchStr)
        return NULL;

    char * lpChStr = WideCharToMultiChar(wchStr, len, &len);
    if (NULL == lpChStr)
    {
        return NULL;
    }

    *outLength = len;
    return lpChStr;

}


std::string CStringConvertion::UTF8StringToMultiChar(const std::string & str)
{
    int len = 0;
    char* res = UTF8StringToMultiChar(str.c_str(), str.length(), &len);
    return std::string(res, len);
}

wstring CStringConvertion::MultiCharToWideChar(const string & sourceStr)
{
    int length = (int)sourceStr.length();
    if (length <= 0)
        return wstring(L"");
    wchar_t * lpCh = MultiCharToWideChar(sourceStr.c_str(), length, &length);
    if (NULL == lpCh)
        return L"";
    else
        return wstring(lpCh);
}


string CStringConvertion::WideCharToMultiChar(const wstring & sourceStr)
{
    int length = (int)sourceStr.length();
    if (length <= 0)
        return string("");

    char * lpCh = WideCharToMultiChar(sourceStr.c_str(), length, &length);
    if (NULL == lpCh)
        return string("");
    else
        return string(lpCh);

}

string CStringConvertion::WideCharToUTF8Char(const wstring & sourceStr)
{
    int length = (int)sourceStr.length();
    if (length <= 0)
        return string("");

    char * lpCh = WideCharToUTF8String(sourceStr.c_str(), length, &length);
    if (NULL == lpCh)
        return string("");
    else
        return string(lpCh, length);

}

char * CStringConvertion::TCHARToUTF8String(const TCHAR * lpSrc, int length, int * outLength)
{

#if (defined(UNICODE) || defined(_UNICODE))

    return WideCharToUTF8String(lpSrc, length, outLength);
#else
    return MultiCharToUTF8String(lpSrc, length, outLength);
#endif

}

TCHAR * CStringConvertion::UTF8StringToTCHAR(char * lpSrc, int length, int *outLength)
{
#if (defined(UNICODE) || defined(_UNICODE))

    return UTF8StringToWideChar(lpSrc, length, outLength);
#else

    return UTF8StringToMultiChar(lpSrc, length, outLength);
#endif

}

#ifdef USE_CSTRING

string CStringConvertion::CStringToString(XCString  src)
{
#if (defined(UNICODE) || defined(_UNICODE))
    LPTSTR lpStr = src.GetBuffer();
    wstring wstr = lpStr;
    src.ReleaseBuffer();
    return WideCharToMultiChar(wstr);

#else
    string str = src.GetBuffer();
    src.ReleaseBuffer();
    return str;
#endif


}
#endif


#ifdef USE_CSTRING

wstring CStringConvertion::CStringToWstring(XCString src)
{
#if (defined(UNICODE) || defined(_UNICODE))
    wstring wstr = src.GetBuffer();
    src.ReleaseBuffer();
    return wstr;
#else
    string str = src.GetBuffer();
    src.ReleaseBuffer();
    return MultiCharToWideChar(str);
#endif
}

#endif


#ifdef USE_CSTRING

XCString CStringConvertion::StringToCString(const string & src)
{

#if (defined(UNICODE) || defined(_UNICODE))
    wstring wstr = MultiCharToWideChar(src);
    return XCString(wstr.c_str());
#else
    return XCString(src.c_str());
#endif

}
#endif


#ifdef USE_CSTRING

XCString CStringConvertion::WStringToCString(const wstring & src)
{
#if (defined(UNICODE) || defined(_UNICODE))
    return XCString(src.c_str());
#else
    string str = WideCharToMultiChar(src);
    return XCString(str.c_str());
#endif
}
#endif


