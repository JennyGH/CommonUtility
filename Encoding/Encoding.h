#pragma once
#include <string>

class Encoder;
class Encoding
{
	Encoding() = delete;
	~Encoding() = default;
public:
	static Encoder GBK;
	static Encoder UTF8;
	static Encoder Default;
};

class Encoder
{
	Encoder(int codePage);
	friend class Encoding;
public:
	Encoder(const Encoder& that);
	Encoder& operator= (const Encoder& that);
	~Encoder();
	std::string GetString(const std::string& ascii) const;
	std::wstring GetString(const std::wstring& unicode) const;
public:
	const int& CodePage;
private:
	int m_codePage;
};
