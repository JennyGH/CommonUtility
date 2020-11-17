#include "Encoding.h"
#include <stdio.h>

int main(int argc, char *argv[])
try
{
    {
        std::string str = "哈哈哈哈a";
        std::string utf8 = Encoding::UTF8.GetString(str);
        std::string ascii = Encoding::GBK.GetString(utf8);
        printf("MultiBytes char utf8: %s, length: %ld\n", utf8.c_str(), utf8.length());
        printf("MultiBytes char ascii: %s, length: %ld\n", ascii.c_str(), ascii.length());
    }
    {
        std::wstring str = L"哈哈哈哈a";
        std::wstring utf8 = Encoding::UTF8.GetString(str);
        std::wstring ascii = Encoding::GBK.GetString(utf8);
        printf("Wide char utf8: %ls, length: %ld\n", utf8.c_str(), utf8.length());
        printf("Wide char ascii: %ls, length: %ld\n", ascii.c_str(), ascii.length());
    }
    return 0;
}
catch (const std::exception& ex)
{
    printf("std::exception: %s\n", ex.what());
    return -1;
}