#include "Guid.h"

int main(int argc, char *argv[])
{
	Guid guid1;
	std::string strGuid = guid1;
	Guid guid2(strGuid);
	std::string guid1Bytes = guid1.GetBytes();
	return 0;
}