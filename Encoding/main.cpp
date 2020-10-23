#include "Encoding.h"

int main(int argc, char *argv[])
{
	{
		std::string str = "哈哈哈哈a";
		auto utf8 = Encoding::UTF8.GetString(str);
		auto ascii = Encoding::GBK.GetString(utf8);
	}
	{
		std::wstring str = L"哈哈哈哈a";
		auto utf8 = Encoding::UTF8.GetString(str);
		auto ascii = Encoding::GBK.GetString(utf8);
	}
	return 0;
}