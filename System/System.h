#pragma once
#include <string>

class System
{
public:
    static int         GetLastError();
    static std::string GetErrorMessage(int errcode);
    static std::string GetLastErrorMessage();
};

