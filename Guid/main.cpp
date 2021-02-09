#include "Guid.h"
#include <string>
#include <iostream>

#define _STDOUT(target) std::cout << #target << ": " << target << std::endl

int main(int argc, char* argv[])
{
    Guid        guid1;
    std::string strGuid = guid1;
    Guid        guid2(strGuid);
    std::string guid1Bytes = guid1.GetBytes();

    _STDOUT(strGuid);

    return 0;
}