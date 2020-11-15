#pragma once
#include <string>
class Convert
{
	Convert() {};
	~Convert() {};
public:
	static std::string ToHex(const unsigned char src[], int len, bool upperCase = false);
	static std::string FromHex(const std::string& hex);
	static std::string ToBase64(const std::string& rawData);
	static std::string ToBase64(const unsigned char src[], int len);
	static std::string FromBase64(const std::string& base64);
};

