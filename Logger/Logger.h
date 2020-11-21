#pragma once
#include <string>
#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_ERROR 3
class Logger
{
    Logger();
public:
    static Logger& GetInstance();
    ~Logger();

    void WriteLog(int level, const char* format, ...);

private:
    FILE* m_pFile;
    int   m_nLevel;
};

class __FunctionTracer__
{
public:
    __FunctionTracer__(const std::string& func);
    ~__FunctionTracer__();
private:
    std::string m_func;
};

#define LOG_INFO(fmt, ...)  Logger::GetInstance().WriteLog(LOG_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::GetInstance().WriteLog(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) Logger::GetInstance().WriteLog(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define TRACE __FunctionTracer__ __trace__(__FUNCTION__)