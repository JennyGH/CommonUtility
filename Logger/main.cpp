#include "Logger.h"
#include <string>
#include <Windows.h>
#include <mutex>
#include <condition_variable>
class __CallStackTracer__
{
public:
    __CallStackTracer__(const std::string& functionName);
    ~__CallStackTracer__();
private:
    std::string m_functionName;
};

__CallStackTracer__::__CallStackTracer__(const std::string& functionName) : m_functionName(functionName)
{
    LOG_TRACE(">>>>> %s", m_functionName.c_str());
}

__CallStackTracer__::~__CallStackTracer__()
{
    LOG_TRACE("<<<<< %s", m_functionName.c_str());
}

#ifdef __PRETTY_FUNCTION__
#define TRACE __CallStackTracer__ __trace__(__PRETTY_FUNCTION__)
#else
#define TRACE __CallStackTracer__ __trace__(__FUNCTION__)
#endif // __PRETTY_FUNCTION__


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

    //std::mutex mutex;
    //std::unique_lock<std::mutex> lock(mutex);
    //std::condition_variable condition;
    //condition.wait(lock, []() -> bool {});

    int rv = 0;
    FILE* pFile = stdout;//_fsopen(".\\logger_test.log", "a", _SH_DENYWR);
    easy_logger_initialize("$stdout", LOG_LEVEL_TRACE);
    test_function_2();
    //easy_logger_end();
    return 0;
}