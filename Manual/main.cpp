#include "Manual.hpp"
#include <string>
#include <iostream>

#define _STDOUT(content) std::cout << #content << ":  " << content << std::endl

int main(int argc, char* argv[])
{
    manual manual(argv[0]);
    manual.push_back("ARG_1", false, 0, "This is optional arg with default value.");
    manual.push_back("ARG_2", true, "This is required arg.");
    _STDOUT(manual.serialize());
    return 0;
}