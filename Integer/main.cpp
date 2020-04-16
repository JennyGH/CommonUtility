#include "Integer.hpp"

int main(int argc, char *argv[])
{
	Int64 a("1024");
	Short max = Short::Min;
	std::string str2(a);
	printf("%s", str2.c_str());
	return 0;
}