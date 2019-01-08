// CommonUtility.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "String.hpp"
#include "StringBuilder.hpp"
#include "StringConvertion.h"
#include "DynamicBuffer.hpp"
#include "Scopes.hpp"

void myforeach(wchar_t& c)
{
	c = ::towlower(c);
}

int main()
{
	common::text::string<wchar_t> wstr(L"dubingjian|123456|杜炳坚");
	wstr.toupper();
	std::list<common::text::string<wchar_t>> res;
	wstr.split(L"\n", res);

	common::text::StringBuilder<wchar_t> builder;
	std::list<common::text::string<wchar_t>>::const_iterator iter = res.begin();
	std::list<common::text::string<wchar_t>>::const_iterator end = res.end();
	for (; iter != end; ++iter)
	{
		builder.appendLine(*iter);
	}
	common::text::string<wchar_t> temp(builder);

	common::text::StringConvertion conv;
	common::text::string<char> str = conv.WideCharToMultiChar(wstr);

	common::raii::DynamicBuffer<char> buffer;
	common::raii::DynamicBuffer<wchar_t> wbuffer;

	wstr.replace(L"|", L";");

	wstr.foreach(myforeach);

	return 0;
}