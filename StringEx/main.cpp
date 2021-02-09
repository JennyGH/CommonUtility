#include "String.hpp"
#include <string>
#include <iostream>

int main(int argc, char* argv[])
{
    std::string_ex::splited_t splited = std::string_ex("你好   我叫   张三   ").split("   ");
    std::size_t                 size    = splited.size();
    for (std::size_t i = 0; i < size; i++)
    {
        std::cout << "splited[" << i << "]: " << (splited[i]) << std::endl;
    }
    return 0;
}