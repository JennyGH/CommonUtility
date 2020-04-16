#include "ArgumentParser.h"

int main(int argc, char *argv[])
try
{
    ArgumentParser arguments(argc, argv);

    auto integer = arguments.get<int>("int", 0);
    auto doubleValue = arguments.get<double>("double", 0.00);
    auto boolean = arguments.get<bool>("boolean", false);
    auto string = arguments.get<std::string>("string", "");
    auto not_exist = arguments.get<std::string>("not-exist");

    return 0;
}
catch (const ArgumentNotFoundException &ex)
{
    Manual manual;
    manual.Add("int", false, "0", "Integer option");
    manual.Add("double", false, "0", "Double option");
    manual.Add("boolean", false, "0", "Boolean option");
    manual.Add("string", false, "0", "String option");
    manual.Add("not-exist", true, "0", "Required option");

    printf(
        "[ArgumentNotFoundException]: %s\r\n"
        "Manual: \r\n"
        "%s",
        ex.what(), manual.ToString().c_str());

    return -1;
}
catch (const std::exception &ex)
{
    printf("[std::exception]: %s\r\n", ex.what());
    return -1;
}