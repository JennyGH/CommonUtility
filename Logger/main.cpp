#include "Logger.h"
#include <string>

class __CallStackTracer__
{
public:
    __CallStackTracer__(const std::string& functionName);
    ~__CallStackTracer__();
};

__CallStackTracer__::__CallStackTracer__(const std::string& functionName)
{
    easy_logger_enter_group(functionName.c_str());
}

__CallStackTracer__::~__CallStackTracer__()
{
    easy_logger_leave_group();
}

#define TRACE __CallStackTracer__ __trace__(__FUNCTION__)

static void test_function_1()
{
    TRACE;
    LOG_DEBUG("TEST!!!");
    LOG_DEBUG("TEST!!!");
    LOG_DEBUG("TEST!!!");
}

static void test_function_2()
{
    TRACE;
    LOG_DEBUG("TEST!!!");
    LOG_DEBUG("TEST!!!");
    test_function_1();
    LOG_DEBUG("TEST!!!");
}

int main(int argc, char* argv[])
{
    int rv = 0;
    rv = easy_logger_initialize(LOG_LEVEL_DEBUG, stdout);
    rv = easy_logger_set_indent_size(4);
    rv = easy_logger_set_indent_content("-");
    test_function_1();
    test_function_2();
    return 0;
}