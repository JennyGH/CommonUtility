#pragma once
#include <cstdint>
class ErrorHandler
{
public:
    ErrorHandler(UINT64 errcode);
    ErrorHandler(const ErrorHandler& that);
    ~ErrorHandler();
public:
    UINT64 errcode;
    char errmsg[1024];
};

