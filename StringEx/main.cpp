#include "String.hpp"

int main(int argc, char *argv[])
{
    std::string_ex::split_res_t splited = std::string_ex("你好   我叫   张三   ").split("   ");
    return 0;
}