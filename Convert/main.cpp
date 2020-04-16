#include "Convert.h"

int main(int argc, char *argv[])
{
	unsigned long longvalue = 0x09abcdef;
	const unsigned char bytes[] = { 0xff, 0x82, 0x16, 0x00 };
	std::string base64 = Convert::ToBase64("BC");
	std::string res = Convert::FromBase64(base64);
	std::string hex = Convert::ToHex((const unsigned char*)&longvalue, sizeof(longvalue));
	std::string raw = Convert::FromHex(hex);

	const char* ptr = raw.c_str();
	longvalue = *((unsigned long*)ptr);
	return 0;
}