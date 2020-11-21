#include <stdio.h>
#include <stdarg.h>
#include "Logger.h"
#include "DateTime.h"
#include "Configuration.h"

#if !defined(WIN32) && !defined(_WIN32)
#ifndef vfprintf_s
#define vfprintf_s vfprintf
#endif // !vfprintf_s
#endif

Logger::Logger()
    : m_pFile(NULL)
#if defined(DEBUG) || defined(_DEBUG)
    , m_nLevel(0)
#else
    , m_nLevel(CONFIGURATIONS.GetLogValue<int>("level", TEST_DEFAULT_CONFIG_LOG_LEVEL))
#endif
{
#if !LOG_TO_CONSOLE
    std::string logDir = CONFIGURATIONS.GetLogValue<std::string>("path", TEST_DEFAULT_CONFIG_LOG_PATH);
    std::string logFileName = DateTime::Now().ToLongDateString() + ".log";
    std::string logFilePath = logDir + "/" + logFileName;
    printf("LOG LEVEL: %d \n", m_nLevel);
    printf("LOG PATH:  %s \n", logFilePath.c_str());
    if (!logFilePath.empty())
    {
        m_pFile = fopen(logFilePath.c_str(), "a");
    }
#endif // !LOG_TO_CONSOLE
}

Logger & Logger::GetInstance()
{
    static Logger* ptr = NULL;
    if (NULL == ptr)
    {
        ptr = new Logger();
    }
    return *ptr;
}

Logger::~Logger()
{
#if !LOG_TO_CONSOLE
    if (NULL != m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
#endif // !LOG_TO_CONSOLE
}

void Logger::WriteLog(int level, const char * format, ...)
{
    if (NULL == format)
    {
        return;
    }

    if (level < m_nLevel)
    {
        return;
    }

    std::string fmt;
    fmt.append(DateTime::Now().ToLongTimeString());
    switch (level)
    {
    case LOG_LEVEL_TRACE: {
        fmt.append(" [TRACE] ");
        break;
    }
    case LOG_LEVEL_DEBUG: {
        fmt.append(" [DEBUG] ");
        break;
    }
    case LOG_LEVEL_INFO: {
        fmt.append(" [INFO]  ");
        break;
    }
    case LOG_LEVEL_ERROR: {
        fmt.append(" [ERROR] ");
        break;
    }
    default:
        break;
    }
    fmt.append(format);
    fmt.append("\n");
    format = fmt.c_str();

    va_list args;
    va_start(args, format);
#if LOG_TO_CONSOLE
    vprintf_s(format, args);
#else
    if (NULL != m_pFile)
    {
        vfprintf_s(m_pFile, format, args);
        fflush(m_pFile);
    }
#endif // LOG_TO_CONSOLE
    va_end(args);
}

__FunctionTracer__::__FunctionTracer__(const std::string& func) :
    m_func(func)
{
    Logger::GetInstance().WriteLog(LOG_LEVEL_TRACE, ">>>>>> %s", m_func.c_str());
}

__FunctionTracer__::~__FunctionTracer__()
{
    Logger::GetInstance().WriteLog(LOG_LEVEL_TRACE, "<<<<<< %s", m_func.c_str());
}
