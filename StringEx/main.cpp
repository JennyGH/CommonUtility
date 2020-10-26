#include "String.hpp"

int main(int argc, char *argv[])
{
	auto splited = std::string_ex("你好   我叫   张三   ").split("   ");
	return 0;
}