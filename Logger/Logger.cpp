#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <deque>
#include <string>
#include "Logger.h"
#include "ConditionVariant.h"

#define USE_MULTI_THREAD 1

// ========== Compatible ==========
#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#include <process.h>
#else
#include <pthread.h>
#ifndef vfprintf_s
#define vfprintf_s                  vfprintf
#endif // !vfprintf_s
#ifndef sprintf_s
#define sprintf_s(buffer, fmt, ...) sprintf(buffer, fmt, ##__VA_ARGS__)
#endif // !sprintf_s
#ifndef GetCurrentThreadId
#define GetCurrentThreadId          pthread_self
#endif // !GetCurrentThreadId
#ifndef localtime_s
#define localtime_s(_Tm, _Time)     localtime_r((_Time), (_Tm))
#endif // !localtime_s
#ifndef gmtime_s
#define gmtime_s(_Tm, _Time)        gmtime_r((_Time), (_Tm))
#endif // !gmtime_s
#endif
// ================================

static FILE* _OpenFile(const char* path, const char* mode)
{
#ifdef WIN32
    return  _fsopen(path, mode, _SH_DENYWR);
#else
    return fopen(path, mode);
#endif // WIN32
}

class ScopeLock
{
public:
    ScopeLock(CRITICAL_SECTION& cs)
        : m_criticalSection(cs)
    {
        ::EnterCriticalSection(&m_criticalSection);
    }
    ~ScopeLock()
    {
        ::LeaveCriticalSection(&m_criticalSection);
    }
private:
    CRITICAL_SECTION& m_criticalSection;
};

typedef std::deque<std::string> LogQueue;
class LoggerImplement
{
    LoggerImplement()
        : m_pFile(NULL)
        , m_nLevel(0)
        , m_bIsLocalFile(true)
        , m_bStop(false)
        , m_hThread(0)
    {
        ::InitializeCriticalSection(&m_mutex);
#if USE_MULTI_THREAD
#if defined WIN32
        m_hThread = (HANDLE)::_beginthreadex(NULL, 0, _WorkThread, this, 0, NULL);
#else
        typedef void*(*thread_function_t)(void*);
        // 线程 1: 获取随机数
        pthread_create(&m_hThread, NULL, (thread_function_t)_WorkThread, this);
        // 线程 2: 对称加解密运算
        pthread_detach(m_hThread);
#endif // defined WIN32
#endif // USE_MULTI_THREAD
    }
private:
    static bool _HasLogContent(void* context)
    {
        LoggerImplement* logger = static_cast<LoggerImplement*>(context);
        return NULL != logger && (logger->m_bStop || !logger->m_logContents.empty());
    }
#if WIN32
    static unsigned int __stdcall _WorkThread(HANDLE arg)
#else
    static void* _WorkThread(void* arg)
#endif // WIN32
    {
        LoggerImplement* logger = static_cast<LoggerImplement*>(arg);
        bool isStop = false;
        while (!isStop)
        {
            LogQueue queue;
            {
                ScopeLock lock(logger->m_mutex);
                logger->m_conditionVariant.Wait(&logger->m_mutex, _HasLogContent, arg);
                logger->m_logContents.swap(queue);
                isStop = logger->m_bStop;
            }

            while (!queue.empty())
            {
                std::string& content = queue.front();
                if (NULL != logger->m_pFile)
                {
                    ::fwrite(content.c_str(), 1, content.length(), logger->m_pFile);
                }
                queue.pop_front();
            }
            if (NULL != logger->m_pFile)
            {
                ::fflush(logger->m_pFile);
            }
        }
        return 0;
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

    void SetFile(FILE* file, bool isStdOut = false)
    {
        this->m_pFile = file;
        this->m_bIsLocalFile = !isStdOut;
    }
    FILE* GetFile() const
    {
        return this->m_pFile;
    }

    void SetLevel(int level)
    {
        this->m_nLevel = level;
    }

    int GetLevel() const
    {
        return this->m_nLevel;
    }

    void SubmitLogContent(const std::string& content)
    {
#if USE_MULTI_THREAD
        {
            ScopeLock lock(m_mutex);
            m_logContents.push_back(content);
        }
        m_conditionVariant.Notify();
#else
        {
            ScopeLock lock(m_mutex);
            if (NULL != m_pFile)
            {
                ::fwrite(content.c_str(), 1, content.length(), m_pFile);
                ::fflush(m_pFile);
            }
        }
#endif USE_MULTI_THREAD
    }

    void Stop()
    {
#if USE_MULTI_THREAD
        if (!IsStop())
        {
            {
                ScopeLock lock(m_mutex);
                this->m_bStop = true;
            }
            m_conditionVariant.Notify();
#if WIN32
            if (WAIT_OBJECT_0 == ::WaitForSingleObject(m_hThread, INFINITE))
            {
                ::CloseHandle(m_hThread);
            }
#else
            pthread_join(m_hThread);
#endif // WIN32
        }
#endif // USE_MULTI_THREAD
        if (m_bIsLocalFile && NULL != m_pFile)
        {
            ::fclose(m_pFile);
            m_pFile = NULL;
        }
    }

    bool IsStop()
    {
        ScopeLock lock(m_mutex);
        return this->m_bStop;
    }

    ~LoggerImplement()
    {
        Stop();
        ::DeleteCriticalSection(&m_mutex);
    }

private:
    FILE*                       m_pFile;
    int                         m_nLevel;
    bool                        m_bIsLocalFile;
    bool                        m_bStop;
    CRITICAL_SECTION            m_mutex;
    ConditionVariant            m_conditionVariant;
    LogQueue                    m_logContents;
#if WIN32
    HANDLE                      m_hThread;
#else
    pthread_t                   m_hThread;
#endif // WIN32
};

void easy_logger_initialize(const char* path, int level)
{
    LoggerImplement& logger = LoggerImplement::GetInstance();
    if (NULL != path && strlen(path) > 0)
    {
        if (strcmp(path, "$stdout") == 0 || strcmp(path, "$STDOUT") == 0)
        {
            LoggerImplement::GetInstance().SetFile(stdout, true);
        }
        else
        {
            LoggerImplement::GetInstance().SetFile(_OpenFile(path, "a+"));
        }
    }
    LoggerImplement::GetInstance().SetLevel(level);
}

int easy_logger_write_log(int level, const char* format, ...)
{
    int bufferSize = 0;
    FILE* pFile = LoggerImplement::GetInstance().GetFile();
    int   nLevel = LoggerImplement::GetInstance().GetLevel();

    if (NULL == pFile)
    {
        return bufferSize;
    }

    if (level < nLevel)
    {
        return bufferSize;
    }

    if (NULL == format)
    {
        return bufferSize;
    }

    std::string fmt;

    // Get current thread id.
    {
        char tid[32] = { 0 };
        sprintf_s(tid, "%ld ", GetCurrentThreadId());
        fmt.append(tid);
    }

    // Generate date time string.
    {
        tm temp;
        time_t tick = time(NULL);
        localtime_s(&temp, &tick);
        char dataTime[32] = { 0 };
        sprintf_s(
            dataTime,
            "%04d%02d%02d%02d%02d%02d",
            temp.tm_year + 1900, temp.tm_mon + 1, temp.tm_mday, temp.tm_hour, temp.tm_min, temp.tm_sec
        );
        fmt.append(dataTime);

    }

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
    if (NULL != pFile)
    {
        bufferSize = vsnprintf(NULL, 0, format, args) + 1;
        char* buffer = new char[bufferSize]();
        if (NULL == buffer)
        {
            return 0;
        }
        vsprintf_s(buffer, bufferSize, format, args);
        LoggerImplement::GetInstance().SubmitLogContent(buffer);
        delete[] buffer;
    }
    va_end(args);

    return bufferSize;
}

void easy_logger_end()
{
    LoggerImplement::GetInstance().Stop();
}
