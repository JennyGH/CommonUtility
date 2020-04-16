#include <string>
#include "Any.hpp"
int main(int argc, char *argv[])
{
	try
	{
		any any = (const char *)"dddd";
		bool bHasValue = any.has_value();
		const std::type_info &info = any.type();
		std::string val = any.cast<const char *>();
		any.reset();
	}
	catch (const bad_any_cast &ex)
	{
		printf(ex.what());
	}
	return 0;
}