#include "pch.h"
#include "Encoding.h"

#include <Windows.h>

Encoder Encoding::UTF8(CP_UTF8);
Encoder Encoding::ASCII(CP_ACP);
Encoder Encoding::Default(CP_ACP);

static char * _WideCharToMultiChar(const wchar_t * lpSrc, int length, int* outLength);
static char * _WideCharToUTF8String(const wchar_t * lpSrc, int length, int* outLength);
static wchar_t * _MultiCharToWideChar(const char * lpSrc, int length, int* outLength);
static char * _MultiCharToUTF8String(const char * lpSrc, int length, int* outLength);
static wchar_t * _UTF8StringToWideChar(const char * lpSrc, int length, int* outLength);
static char * _UTF8StringToMultiChar(const char * lpSrc, int length, int* outLength);

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

Encoder& Encoder::operator= (const Encoder& that)
{
	if (&that != this)
	{
		m_codePage = that.m_codePage;
	}
	return *this;
}

const Encoder::TString& Encoder::GetString(const std::string & str) const
{
	return GetString(str.c_str(), str.length());
}

const Encoder::TString& Encoder::GetString(const std::wstring & wstr) const
{
	return GetString(wstr.c_str(), wstr.length());
}

const Encoder::TString& Encoder::GetString(const char src[], int len) const
{
	static TString res;

	res.clear();

	switch (m_codePage)
	{
	case CP_ACP:
	{
		//Convert to ASCII
#ifdef UNICODE
		int wcLen = 0;
		wchar_t* wc = _UTF8StringToWideChar(src, len, &wcLen);
		if (NULL == wc)
		{
			break;
		}
		res.assign(wc, wcLen);
		delete[] wc;
#else
		int mcLen = 0;
		char* mc = _UTF8StringToMultiChar(src, len, &mcLen);
		if (NULL == mc)
		{
			break;
		}
		res.assign(mc, mcLen);
		delete[] mc;
		mc = NULL;
#endif // UNICODE
		break;
	}
	case CP_UTF8:
	{
		//Convert to UTF8
		int mcLen = 0;
		char* mc = _MultiCharToUTF8String(src, len, &mcLen);
		if (NULL == mc)
		{
			break;
		}
#ifdef UNICODE
		//Convert to WideChar
		int wcLen = 0;
		wchar_t* wc = _MultiCharToWideChar(mc, mcLen, &wcLen);
		if (NULL == wc)
		{
			delete[] mc;
			break;
		}
		res.assign(wc, wcLen);
		delete[] wc;
#else
		//No convert
		res.assign(mc, mcLen);
#endif // UNICODE
		delete[] mc;
		break;
	}
	default:
		break;
	}

	return res;
}

const Encoder::TString& Encoder::GetString(const wchar_t src[], int len) const
{
	static TString res;
	res.clear();

	switch (m_codePage)
	{
	case CP_ACP:
	{
		//Convert to ASCII
#ifdef UNICODE
		int mcLen = 0;
		char* mc = _WideCharToMultiChar(src, len, &mcLen);
		if (NULL == mc)
		{
			break;
		}

		int wcLen = 0;
		wchar_t* wc = _UTF8StringToWideChar(mc, mcLen, &wcLen);
		if (NULL == wc)
		{
			delete mc;
			break;
		}

		res.assign(wc, wcLen);

		delete[] mc;
		delete[] wc;
#else
		int mc1Len = 0;
		char* mc1 = _WideCharToMultiChar(src, len, &mc1Len);
		if (NULL == mc1)
		{
			break;
		}
		int mc2Len = 0;
		char* mc2 = _UTF8StringToMultiChar(mc1, mc1Len, &mc2Len);
		if (NULL == mc2)
		{
			delete[] mc2;
			break;
		}
		res.assign(mc2, mc2Len);
		delete[] mc1;
		delete[] mc2;
#endif // UNICODE
		break;
	}
	case CP_UTF8:
	{
		//Convert to UTF8
#ifdef UNICODE
		int mcLen = 0;
		char* mc = _WideCharToUTF8String(src, len, &mcLen);
		if (NULL == mc)
		{
			break;
		}
		int wcLen = 0;
		wchar_t* wc = _MultiCharToWideChar(mc, mcLen, &wcLen);
		if (NULL == wc)
		{
			delete[] mc;
			break;
		}
		res.assign(wc, wcLen);
		delete[] mc;
		delete[] wc;
#else
		int mcLen = 0;
		char* mc = _WideCharToUTF8String(src, len, &mcLen);
		if (NULL == mc)
		{
			break;
		}
		res.assign(mc, mcLen);
		delete[] mc;
#endif // UNICODE
		break;
	}
	default:
		break;
	}

	return res;
}

const Encoder::TString& Encoder::GetString(const unsigned char src[], int len) const
{
	return GetString((const char*)src, len);
}

static char * _WideCharToMultiChar(const wchar_t * lpSrc, int length, int* outLength)
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

	return chStr;
}

static char * _WideCharToUTF8String(const wchar_t * lpSrc, int length, int* outLength)
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

	return  utfStr;
}

static wchar_t * _MultiCharToWideChar(const char * lpSrc, int length, int* outLength)
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

	return wideCh;
}

static char * _MultiCharToUTF8String(const char * lpSrc, int length, int* outLength)
{
	if (NULL == lpSrc || length <= 0 || outLength == NULL)
		return NULL;

	//First convert the ansi to unicode
	int len = 0;
	wchar_t * wchStr = _MultiCharToWideChar(lpSrc, length, &len);
	if (NULL == wchStr)
		return NULL;

	//Then convert the unicode to utf8
	char * utfStr = _WideCharToUTF8String(wchStr, len, &len);
	if (NULL == utfStr)
	{
		return NULL;
	}
	*outLength = len;
	return utfStr;
}

static wchar_t * _UTF8StringToWideChar(const char * lpSrc, int length, int* outLength)
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

	return wchStr;
}

static char * _UTF8StringToMultiChar(const char * lpSrc, int length, int* outLength)
{
	if (NULL == lpSrc || length <= 0 || outLength == NULL)
		return NULL;

	//First convert utf8 to wide char
	int len = 0;
	wchar_t * wchStr = _UTF8StringToWideChar(lpSrc, length, &len);
	if (NULL == wchStr)
		return NULL;

	char * lpChStr = _WideCharToMultiChar(wchStr, len, &len);
	if (NULL == lpChStr)
	{
		return NULL;
	}

	*outLength = len;
	return lpChStr;

}