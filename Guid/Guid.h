#pragma once
#include <string>

class Guid
{
public:
	Guid();
	Guid(const std::string& guid);
	Guid(const Guid& that);
	Guid& operator= (const Guid& that);
	~Guid();
	operator std::string() const;
	std::string GetBytes() const;
private:
	unsigned long m_a;
	unsigned short m_b;
	unsigned short m_c;
	unsigned char m_d;
	unsigned char m_e;
	unsigned char m_f;
	unsigned char m_g;
	unsigned char m_h;
	unsigned char m_i;
	unsigned char m_j;
	unsigned char m_k;
};

