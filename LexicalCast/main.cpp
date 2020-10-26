#include <string>
#include <iostream>
#include "LexicalCast.hpp"

using namespace std;
void test()
{
	cout << lexical_cast<int>(1) << endl;
	cout << lexical_cast<int>("1") << endl;
	cout << lexical_cast<long>("1") << endl;
	cout << lexical_cast<string>(1) << endl;
	cout << lexical_cast<bool>(1) << endl;
	cout << lexical_cast<double>("1.2") << endl;
	cout << lexical_cast<float>("1.2") << endl;
	string s = "true";
	cout << lexical_cast<bool>(s) << endl;
	char* p = "false";
	cout << lexical_cast<bool>(p) << endl;
	const char* q = "false";
	cout << lexical_cast<bool>(q) << endl;
	cout << lexical_cast<bool>("false") << endl;
	cout << lexical_cast<bool>("test") << endl;
}

int main()
{
	try
	{
		test();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}

	return 0;
}