#include "pch.h"
#include "StringConvertion.h"

namespace common
{
	namespace text
	{
		StringConvertion::StringConvertion(void)
		{
		}

		StringConvertion::~StringConvertion(void)
		{
			std::size_t count = m_utf8Char.size();
			for (std::size_t s = 0; s < count; ++s)
			{
				char * lpChar = m_utf8Char[s];
				if (lpChar)
					delete[] lpChar;
			}
			m_utf8Char.clear();

			count = m_wideChar.size();
			for (std::size_t n = 0; n < count; ++n)
			{
				wchar_t * lpWchar = m_wideChar[n];
				if (lpWchar)
					delete[] lpWchar;
			}

			m_wideChar.clear();

			count = m_multiChar.size();
			for (std::size_t i = 0; i < count; ++i)
			{
				char * lpChar = m_multiChar[i];
				if (lpChar)
					delete[] lpChar;
			}
			m_multiChar.clear();
		}


		char * StringConvertion::WideCharToMultiChar(const wchar_t * lpSrc, int length, int* outLength)
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


		char * StringConvertion::WideCharToUTF8String(const wchar_t * lpSrc, int length, int* outLength)
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


		wchar_t * StringConvertion::MultiCharToWideChar(const char * lpSrc, int length, int* outLength)
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


		char * StringConvertion::MultiCharToUTF8String(const char * lpSrc, int length, int* outLength)
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


		wchar_t * StringConvertion::UTF8StringToWideChar(const char * lpSrc, int length, int* outLength)
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


		char * StringConvertion::UTF8StringToMultiChar(const char * lpSrc, int length, int* outLength)
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


		std::wstring StringConvertion::MultiCharToWideChar(const std::string & sourceStr)
		{
			int length = (int)sourceStr.length();
			if (length <= 0)
				return (L"");
			wchar_t * lpCh = MultiCharToWideChar(sourceStr.c_str(), length, &length);
			if (NULL == lpCh)
				return L"";
			else
				return (lpCh);
		}


		std::string StringConvertion::WideCharToMultiChar(const std::wstring & sourceStr)
		{
			int length = (int)sourceStr.length();
			if (length <= 0)
				return ("");

			char * lpCh = WideCharToMultiChar(sourceStr.c_str(), length, &length);
			if (NULL == lpCh)
				return ("");
			else
				return (lpCh);

		}


		char * StringConvertion::TCHARToUTF8String(const TCHAR * lpSrc, int length, int * outLength)
		{

#if (defined(UNICODE) || defined(_UNICODE))

			return WideCharToUTF8String(lpSrc, length, outLength);
#else
			return MultiCharToUTF8String(lpSrc, length, outLength);
#endif

		}

		TCHAR * StringConvertion::UTF8StringToTCHAR(char * lpSrc, int length, int *outLength)
		{
#if (defined(UNICODE) || defined(_UNICODE))
			return UTF8StringToWideChar(lpSrc, length, outLength);
#else

			return UTF8StringToMultiChar(lpSrc, length, outLength);
#endif

		}

#ifdef USE_CSTRING

		std::string StringConvertion::CStringToString(XCString  src)
		{
#if (defined(UNICODE) || defined(_UNICODE))
			LPTSTR lpStr = src.GetBuffer();
			std::wstring wstr = lpStr;
			src.ReleaseBuffer();
			return WideCharToMultiChar(wstr);
#else
			std::string str = src.GetBuffer();
			src.ReleaseBuffer();
			return str;
#endif
		}
#endif


#ifdef USE_CSTRING

		std::wstring StringConvertion::CStringToWstring(XCString src)
		{
#if (defined(UNICODE) || defined(_UNICODE))
			std::wstring wstr = src.GetBuffer();
			src.ReleaseBuffer();
			return wstr;
#else
			std::string str = src.GetBuffer();
			src.ReleaseBuffer();
			return MultiCharToWideChar(str);
#endif
		}

#endif


#ifdef USE_CSTRING

		XCString StringConvertion::StringToCString(const std::string & src)
		{
#if (defined(UNICODE) || defined(_UNICODE))
			return XCString(MultiCharToWideChar(src).c_str());
#else
			return XCString(src.c_str());
#endif

		}
#endif


#ifdef USE_CSTRING

		XCString StringConvertion::WStringToCString(const std::wstring & src)
		{
#if (defined(UNICODE) || defined(_UNICODE))
			return XCString(src.c_str());
#else
			return XCString(WideCharToMultiChar(src).c_str());
#endif
		}
#endif

		std::wstring StringConvertion::UTF8StringToWstring(char * lpStr)
		{
			int outlen = 0;
			wchar_t * lpBuf = UTF8StringToWideChar(lpStr, strlen(lpStr), &outlen);
			if (lpBuf != NULL)
				return std::wstring(lpBuf, outlen);
			else
				return (L"");
		}

	}
}