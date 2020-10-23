#include "String.hpp"

int main(int argc, char *argv[])
{
	auto splited = common::text::string<char>("你好   我叫   张三   ").split("   ");
	return 0;
}