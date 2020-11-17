#include "ArgumentParser.h"
#include <stdio.h>

int main(int argc, char *argv[])
try
{
    ArgumentParser arguments(argc, argv);

    int integer = arguments.get<int>("int", 0);
    double doubleValue = arguments.get<double>("double", 0.00);
    bool boolean = arguments.get<bool>("boolean", false);
    std::string string = arguments.get<std::string>("string", "");
    std::string not_exist = arguments.get<std::string>("not-exist");

    return 0;
}
catch (const ArgumentNotFoundException &ex)
{
    printf("[ArgumentNotFoundException]: %s\r\n", ex.what());
    return -1;
}
catch (const std::exception &ex)
{
    printf("[std::exception]: %s\r\n", ex.what());
    return -1;
}