#pragma once
#include <string>
class Convert
{
	Convert() {};
	~Convert() {};
public:
	static const std::string& ToHex(const unsigned char src[], int len, bool upperCase = false);
	static const std::string& FromHex(const std::string& hex);
	static const std::string& ToBase64(const std::string& rawData);
	static const std::string& ToBase64(const unsigned char src[], int len);
	static const std::string& FromBase64(const std::string& base64);
};

