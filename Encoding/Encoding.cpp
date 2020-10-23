#include "Encoding.h"
#include <codecvt>

enum CodePage
{
	GBK = 0,
	UTF8
};

Encoder Encoding::GBK(CodePage::GBK);
Encoder Encoding::UTF8(CodePage::UTF8);
Encoder Encoding::Default(CodePage::GBK);

using utf8_convertor_type = std::codecvt_utf8<wchar_t>;
using gbk_convertor_type = std::codecvt_byname<wchar_t, char, std::mbstate_t>;

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
	case CodePage::GBK:
	{
		return wstring_to_string(utf8string_to_wstring(ascii));
	}
	case CodePage::UTF8:
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
	case CodePage::GBK:
	{
		return utf8string_to_wstring(wstring_to_string(unicode));
	}
	case CodePage::UTF8:
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