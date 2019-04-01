#pragma once
#include <string>



class Encoder;
class Encoding
{
	Encoding() {};
	~Encoding() {};
public:
	static Encoder UTF8;
	static Encoder ASCII;
	static Encoder Default;
};

class Encoder
{
#ifdef UNICODE
	typedef std::wstring TString;
#else
	typedef std::string TString;
#endif // UNICODE

#define READONLY(type) const type&

	Encoder(int codePage);
	friend class Encoding;
public:
	Encoder(const Encoder& that);
	Encoder& operator= (const Encoder& that);
	~Encoder();
	const TString& GetString(const std::string& str) const;
	const TString& GetString(const std::wstring& wstr) const;
	const TString& GetString(const char src[], int len) const;
	const TString& GetString(const wchar_t src[], int len) const;
	const TString& GetString(const unsigned char src[], int len) const;
public:
	READONLY(int) CodePage;
private:
	int m_codePage;
};
