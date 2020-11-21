#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <map>
#include <stack>
#include <string>
#include "Logger.h"

// ========== Compatible ==========
#if !defined(WIN32) && !defined(_WIN32)
#ifndef vfprintf_s
#define vfprintf_s                  vfprintf
#endif // !vfprintf_s
#ifndef sprintf_s
#define sprintf_s(buffer, fmt, ...) sprintf(buffer, fmt, ##__VA_ARGS__)
#endif // !sprintf_s
static inline void assign_tm(struct tm* const from, struct tm* const to)
{
    if (NULL == from || NULL == to)
    {
        return;
    }
    to->tm_year = from->tm_year;
    to->tm_mon = from->tm_mon;
    to->tm_mday = from->tm_mday;
    to->tm_wday = from->tm_wday;
    to->tm_yday = from->tm_yday;
    to->tm_hour = from->tm_hour;
    to->tm_min = from->tm_min;
    to->tm_sec = from->tm_sec;
    to->tm_isdst = from->tm_isdst;
}

static inline int localtime_s(struct tm* const t, time_t const* time)
{
    if (NULL == time || NULL == t)
    {
        return 0;
    }
    assign_tm(localtime(time), t);
    return 0;
}

static inline int gmtime_s(struct tm* const t, time_t const* time)
{
    if (NULL == time || NULL == t)
    {
        return 0;
    }
    assign_tm(gmtime(time), t);
    return 0;
}
#endif
// ================================

// ========== Inner functions ==========
static std::string GetDateString(time_t tick)
{
    tm temp;
    localtime_s(&temp, &tick);
    int year = temp.tm_year + 1900;
    int month = temp.tm_mon + 1;
    int day = temp.tm_mday;
    char buffer[256] = { 0 };
    sprintf_s(buffer, "%04d_%02d_%02d", year, month, day);
    return buffer;
}

static std::string GetTimeString(time_t tick)
{
    tm temp;
    localtime_s(&temp, &tick);
    int hour = temp.tm_hour;
    int minute = temp.tm_min;
    int second = temp.tm_sec;
    char buffer[256] = { 0 };
    sprintf_s(buffer, "%02d:%02d:%02d", hour, minute, second);
    return buffer;
}

static std::string GetIdentation(int indentSize, const std::string& indentContent = " ")
{
    std::string res;
    for (int i = 0; i < indentSize; i++)
    {
        res.append(indentContent);
    }
    return res;
}
// =====================================

struct Indentation
{
    std::string group;
    std::string ident;
};

class LoggerImplement
{
    LoggerImplement()
        : m_pFile(NULL)
        , m_nLevel(0)
        , m_nIndentSize(4)
        , m_indentContent(" ")
    {
        //m_pFile = fopen(NULL, "a");
    }
public:
    static LoggerImplement& GetInstance()
    {
        static LoggerImplement* ptr = NULL;
        if (NULL == ptr)
        {
            ptr = new LoggerImplement();
        }
        return *ptr;
    }

    void Close()
    {
        if (NULL != m_pFile)
        {
            fclose(m_pFile);
            m_pFile = NULL;
        }
    }

    void EnterGroup(const std::string& group)
    {
        Indentation indentation;
        indentation.group = group;
        indentation.ident = (this->m_indents.empty() ? "" : this->m_indents.top().ident) + GetIdentation(m_nIndentSize, m_indentContent);
        this->m_indents.push(indentation);
    }
    void LeaveGroup()
    {
        this->m_indents.pop();
    }
    std::string GetCurrentIndent()
    {
        if (this->m_indents.empty())
        {
            return "";
        }
        return this->m_indents.top().ident;
    }
    std::string GetCurrentGroup()
    {
        std::string group;
        if (!this->m_indents.empty())
        {
            group = this->m_indents.top().group;
        }
        return group;
    }

    ~LoggerImplement()
    {
        this->Close();
    }

public:
    FILE* m_pFile;
    int                       m_nLevel;
    int                       m_nIndentSize;
    std::string               m_indentContent;
    std::stack<Indentation>   m_indents;
};

int easy_logger_initialize(int level, FILE* pFile)
{
    if (NULL == pFile)
    {
        return EASY_LOGGER_ERROR_INVALID_ARGUMENT;
    }
    LoggerImplement::GetInstance().m_nLevel = level;
    LoggerImplement::GetInstance().m_pFile = pFile;

    return EASY_LOGGER_SUCCESS;
}

//int easy_logger_initialize(int level, const char* path)
//{
//    if (NULL == path || strlen(path) == 0)
//    {
//        return EASY_LOGGER_ERROR_INVALID_ARGUMENT;
//    }
//
//    FILE* pFile = fopen(path, "a");
//
//    if (NULL == pFile)
//    {
//        return EASY_LOGGER_ERROR_NO_ACCESS_RIGHT;
//    }
//
//    LoggerImplement::GetInstance().m_pFile = pFile;
//
//    return EASY_LOGGER_SUCCESS;
//}

int easy_logger_set_indent_size(int size)
{
    LoggerImplement::GetInstance().m_nIndentSize = size;
    return EASY_LOGGER_SUCCESS;
}

int easy_logger_set_indent_content(const char* content)
{
    if (NULL == content)
    {
        return EASY_LOGGER_ERROR_INVALID_ARGUMENT;
    }
    LoggerImplement::GetInstance().m_indentContent = content;
    return EASY_LOGGER_SUCCESS;
}

int easy_logger_enter_group(const char* group)
{
    if (NULL == group || strlen(group) == 0)
    {
        return EASY_LOGGER_ERROR_INVALID_ARGUMENT;
    }
    easy_logger_write_log(LOG_LEVEL_TRACE, ">>>>> %s", group);
    LoggerImplement::GetInstance().EnterGroup(group);
    return EASY_LOGGER_SUCCESS;
}

int easy_logger_leave_group()
{
    std::string group = LoggerImplement::GetInstance().GetCurrentGroup();
    LoggerImplement::GetInstance().LeaveGroup();
    easy_logger_write_log(LOG_LEVEL_TRACE, "<<<<< %s", group.c_str());
    return EASY_LOGGER_SUCCESS;
}

int easy_logger_write_log(int level, const char* format, ...)
{
    FILE* pFile = LoggerImplement::GetInstance().m_pFile;
    int   nLevel = LoggerImplement::GetInstance().m_nLevel;

    if (NULL == format)
    {
        return EASY_LOGGER_ERROR_INVALID_ARGUMENT;
    }

    if (level < nLevel)
    {
        return EASY_LOGGER_SUCCESS;
    }

    std::string fmt;
    fmt.append(GetTimeString(time(NULL)));
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
    fmt.append(LoggerImplement::GetInstance().GetCurrentIndent());
    fmt.append(format);
    fmt.append("\n");
    format = fmt.c_str();

    va_list args;
    va_start(args, format);
    if (NULL != pFile)
    {
        vfprintf_s(pFile, format, args);
        fflush(pFile);
    }
    va_end(args);

    return EASY_LOGGER_SUCCESS;
}

int easy_logger_close()
{
    LoggerImplement::GetInstance().Close();
    return EASY_LOGGER_SUCCESS;
}