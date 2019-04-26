// convertion_test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "Integer.hpp"
#include "Registry.h"
#include "Encoding.h"
#include "Convert.h"
#include "Guid.h"
#include "Any.hpp"
#include "DateTime.h"
#include "LogStream.h"

#define TEST_ANY 0
#define TEST_GUID 0
#define TEST_CONVERT 0
#define TEST_ENCODING 0
#define TEST_REGISTRY 0
#define TEST_INTEGER 0
#define TEST_DATETIME 0
#define TEST_LOGSTREAM 0

int main()
{
	//LogStreamWrapper::Get().SetLevel(Info);
	//LogStreamWrapper::Get().SetPath("temp.log");
	LOG_DEBUG << 123;
	LOG_DEBUG << "dubingjian";
	LOG_INFO << "dubingjian";
	LOG_ERROR << "dubingjian";

#if TEST_DATETIME
	DateTime now = DateTime::Now();
	DateTime today = DateTime::Today();
	DateTime utc = DateTime::UtcNow();
	DateTime local = utc.ToLocalTime();
	DateTime universal = now.ToUniversalTime();
	std::string longDateString = now.ToLongDateString();
	std::string longTimeString = now.ToLongTimeString();
	std::string shortDateString = now.ToShortDateString();
	std::string shortTimeString = now.ToShortTimeString();
	std::string strDateTime = now.ToString("%Y-%m-%d %H:%M:%S %a");

	TimeSpan::MaxValue;
	TimeSpan::MinValue;
	TimeSpan timeSpan0(1, 2, 3, 4, 5);
	TimeSpan timeSpan1(6, 7, 8, 9, 10);
	TimeSpan timeSpan2(3600 * 1000);
	TimeSpan aaa = timeSpan0 - timeSpan1;
	timeSpan0.Subtract(timeSpan1);
	TimeSpan duration = aaa.Duration();
	TimeSpan negate = aaa.Negate();
#endif // TEST_DATETIME

#if TEST_ANY
	try
	{
		any any = (const char*)"dddd";
		bool bHasValue = any.has_value();
		const std::type_info& info = any.type();
		std::string val = any.cast<const char*>();
		any.reset();
	}
	catch (const bad_any_cast& ex)
	{
		printf(ex.what());
}
#endif // TEST_ANY

#if TEST_GUID
	Guid guid1;
	std::string strGuid = guid1;
	Guid guid2(strGuid);
	std::string guid1Bytes = guid1.GetBytes();
#endif // TEST_GUID

#if TEST_CONVERT
	unsigned long longvalue = 0x09abcdef;
	const unsigned char bytes[] = { 0xff, 0x82, 0x16, 0x00 };
	std::string base64 = Convert::ToBase64("BC");
	std::string res = Convert::FromBase64(base64);
	std::string hex = Convert::ToHex((const unsigned char*)&longvalue, sizeof(longvalue));
	std::string raw = Convert::FromHex(hex);

	const char* ptr = raw.c_str();
	longvalue = *((unsigned long*)ptr);
#endif

#if TEST_ENCODING
	std::string str = "哈哈哈哈";
	std::wstring wstr = L"哈哈哈哈";
	const unsigned char bytes2[] = "哈哈哈哈";
	std::wstring utf8 = Encoding::UTF8.GetString(bytes, sizeof(bytes));
	std::wstring ascii = Encoding::ASCII.GetString(utf8);
	int codePage = Encoding::UTF8.CodePage;
#endif // TEST_ENCODING

#if TEST_REGISTRY
	try
	{
		RegistryKey& key = Registry::LocalMachine;
		RegistryKey& software = key.OpenSubKey(_T("SOFTWARE\\NETCA\\PKI\\Devices\\NETCAKeyMobileKey"), false);
		StringType val = software.GetValue(_T("UserName"), _T(""));
		StringArray valueNames = software.GetValueNames();
		StringArray keyNames = software.GetSubKeyNames();
		software.CreateSubKey(_T("A\\B\\B1"));
		software.CreateSubKey(_T("A\\B\\B2"));
		software.CreateSubKey(_T("A\\B\\B3"));
		software.CreateSubKey(_T("A\\B\\B4"));
		software.CreateSubKey(_T("A\\B\\C"));
		software.CreateSubKey(_T("A\\B\\C\\C1"));
		software.CreateSubKey(_T("A\\B\\C\\C2"));
		software.CreateSubKey(_T("A\\B\\C\\C3"));
		software.CreateSubKey(_T("A\\B\\C\\C4"));
		software.CreateSubKey(_T("A\\B\\C\\D"));
		software.CreateSubKey(_T("A\\B\\C\\D\\D1"));
		software.CreateSubKey(_T("A\\B\\C\\D\\D2"));
		software.CreateSubKey(_T("A\\B\\C\\D\\D3"));
		software.CreateSubKey(_T("A\\B\\C\\D\\D4"));
		software.DeleteSubKeyTree(_T("A\\"));
	}
	catch (const RegistryException& ex)
	{
		_tprintf(ex.What().c_str());
	}
#endif // TEST_REGISTRY

#if TEST_INTEGER
	Int64 a("1024");
	Short max = Short::Min;
	std::string str2(a);
	printf("%s", str.c_str());
#endif // TEST_INTEGER

	return 0;
}