#include "Logger.h"
#include <string>
#include <string.h>
#if WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif // WIN32
class __CallStackTracer__
{
public:
    __CallStackTracer__(const std::string& functionName);
    ~__CallStackTracer__();
private:
    std::string m_functionName;
};

__CallStackTracer__::__CallStackTracer__(const std::string& functionName)
    : m_functionName(functionName)
{
    LOG_TRACE(">>>>> %s", m_functionName.c_str());
}

__CallStackTracer__::~__CallStackTracer__()
{
    LOG_TRACE("<<<<< %s", m_functionName.c_str());
    //printf("<<<<< %s \n", m_functionName.c_str());
}

#define TRACE __CallStackTracer__ __trace__(__FUNCTION__)

static void test_function_1()
{
    TRACE;
    LOG_DEBUG("1");
}

static void test_function_2()
{
    TRACE;
    LOG_DEBUG("2");
    test_function_1();
    LOG_DEBUG("3");
}

int main(int argc, char* argv[])
{
    int rv = 0;
    FILE* pFile = stdout;//_fsopen(".\\logger_test.log", "a", _SH_DENYWR);
    easy_logger_initialize("$stdout", LOG_LEVEL_TRACE);
    test_function_2();
    //easy_logger_end();
    return 0;
}