#include "pch.h"
#include "Convert.h"

static const char g_base64EncodeTable[] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

static const char g_base64DecodeTable[] =
{
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
	-2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
	-2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
};

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

const std::string& Convert::ToBase64(const std::string & rawData)
{
	return ToBase64((const unsigned char*)rawData.data(), rawData.length());
}

const std::string& Convert::ToBase64(const unsigned char src[], int len)
{
	static std::string res;

	res.clear();

	if (NULL == src || len <= 0)
	{
		return res;
	}

	const unsigned char * current = src;
	while (len > 2)
	{
		res += g_base64EncodeTable[current[0] >> 2];
		res += g_base64EncodeTable[((current[0] & 0x03) << 4) + (current[1] >> 4)];
		res += g_base64EncodeTable[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
		res += g_base64EncodeTable[current[2] & 0x3f];
		current += 3;
		len -= 3;
	}
	if (len > 0)
	{
		int mod = len % 3;
		res += g_base64EncodeTable[current[0] >> 2];
		if (mod == 1)
		{
			res += g_base64EncodeTable[(current[0] & 0x03) << 4];
			res += "==";
		}
		else if (mod == 2)
		{
			res += g_base64EncodeTable[((current[0] & 0x03) << 4) + (current[1] >> 4)];
			res += g_base64EncodeTable[(current[1] & 0x0f) << 2];
			res += "=";
		}
	}

	return res;
}

const std::string& Convert::FromBase64(const std::string & base64)
{
	//½âÂë±í
	static std::string res;
	res.clear();

	char ch;

	int bin = 0,
		i = 0,
		pos = 0,
		length = base64.length();

	const char *current = base64.c_str();

	while ((ch = *current++) != '\0' && length-- > 0)
	{
		if (ch == '=')
		{
			if (*current != '=' && (i % 4) == 1)
			{
				return res;
			}
			continue;
		}

		ch = g_base64DecodeTable[ch];

		if (ch < 0)
		{
			continue;
		}
		switch (i % 4)
		{
		case 0:
			bin = ch << 2;
			break;
		case 1:
			bin |= ch >> 4;
			res += bin;
			bin = (ch & 0x0f) << 4;
			break;
		case 2:
			bin |= ch >> 2;
			res += bin;
			bin = (ch & 0x03) << 6;
			break;
		case 3:
			bin |= ch;
			res += bin;
			break;
		}
		i++;
	}

	return res;
}

const std::string & Convert::ToHex(const unsigned char src[], int len, bool upperCase)
{
	static std::string res;
	res.clear();

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

const std::string & Convert::FromHex(const std::string & hex)
{
	static std::string res;
	res.clear();

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
