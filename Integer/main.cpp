#include "Integer.hpp"
#include <string>
#include <cstdlib>
#include <iostream>

#define _STDOUT(content) std::cout << #content << ":  " << content << std::endl

int main(int argc, char *argv[])
{
    _STDOUT(Integer<int>::Max);
    _STDOUT(Integer<int>::Min);
    _STDOUT(Integer<bool>::Max);
    _STDOUT(Integer<bool>::Min);
    _STDOUT(Integer<short>::Max);
    _STDOUT(Integer<short>::Min);
    _STDOUT(Integer<long>::Max);
    _STDOUT(Integer<long>::Min);
    _STDOUT(Integer<long long>::Max);
    _STDOUT(Integer<long long>::Min);
    _STDOUT(Integer<unsigned int>::Max);
    _STDOUT(Integer<unsigned int>::Min);
    _STDOUT(Integer<unsigned short>::Max);
    _STDOUT(Integer<unsigned short>::Min);
    _STDOUT(Integer<unsigned long>::Max);
    _STDOUT(Integer<unsigned long>::Min);
    _STDOUT(Integer<unsigned long long>::Max);
    _STDOUT(Integer<unsigned long long>::Min);
    Int64 a("1024");
    Short max = Short::Min;
    std::string str2(a);
    printf("%s", str2.c_str());
    return 0;
}