#include "Encoding.h"

int main(int argc, char *argv[])
{
	std::string str = "哈哈哈哈";
	std::wstring wstr = L"哈哈哈哈";
	const unsigned char bytes[] = "哈哈哈哈";
	auto utf8 = Encoding::UTF8.GetString(bytes, sizeof(bytes));
	auto ascii = Encoding::ASCII.GetString(utf8);
	int codePage = Encoding::UTF8.CodePage;
	return 0;
}