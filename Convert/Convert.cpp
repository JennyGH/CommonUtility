#include "Convert.h"
#include "Base64.h"

static char GetHex(int val, bool upperCase = false)
{
	if (0 <= val && val <= 9)
	{
		return '0' + val;
	}
	else if (10 <= val && val <= 15)
	{
		if (upperCase)
		{
			return 'A' + (val - 10);
		}
		else
		{
			return 'a' + (val - 10);
		}
	}
	return 0;
}

static unsigned char GetByte(char c)
{
	if ('0' <= c && c <= '9')
	{
		return c - '0';
	}
	else if ('a' <= c && c <= 'f')
	{
		return c - 'a' + 10;
	}
	else if ('A' <= c && c <= 'F')
	{
		return c - 'A' + 10;
	}
	return 0x00;
}

std::string Convert::ToBase64(const std::string& rawData)
{
	return ToBase64((const unsigned char*)rawData.data(), rawData.length());
}

std::string Convert::ToBase64(const unsigned char src[], int len)
{
	return base64_encode(src, len);
}

std::string Convert::FromBase64(const std::string& base64)
{
	return base64_decode(base64);
}

std::string Convert::ToHex(const unsigned char src[], int len, bool upperCase)
{
	std::string res;
	if (NULL == src || len <= 0)
	{
		return res;
	}

	for (int index = 0; index < len; index++)
	{
		char c = src[index];
		char buffer[2] = { 0 };
		buffer[0] = GetHex((c >> 4) & 0x0f);
		buffer[1] = GetHex((c >> 0) & 0x0f);
		res.append(buffer, 2);
	}

	return res;
}

std::string Convert::FromHex(const std::string& hex)
{
	std::string res;
	if (hex.empty() || hex.length() % 2 != 0)
	{
		return res;
	}

	std::string::const_iterator iter = hex.begin();
	std::string::const_iterator end = hex.end();
	for (; iter != end;)
	{
		const char& hc = *iter++;
		const char& lc = *iter++;
		unsigned char byte = 0x00;
		byte |= (GetByte(hc) << 4);
		byte |= (GetByte(lc) << 0);
		res.append(1, byte);
	}

	return res;
}
