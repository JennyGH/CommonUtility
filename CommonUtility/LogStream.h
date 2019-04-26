#pragma once
#include <fstream>
#include <ctime>

#define LOG_DEBUG (LogStreamWrapper::Get().GetStream(Debug) << "[" << __FUNCTION__ << "] ")
#define LOG_INFO  (LogStreamWrapper::Get().GetStream( Info) << "[" << __FUNCTION__ << "] ")
#define LOG_ERROR (LogStreamWrapper::Get().GetStream(Error) << "[" << __FUNCTION__ << "] ")

#ifndef WIN32
#define localtime_s(refTm, refTime) localtime_r(refTime, refTm)
#endif // !WIN32

#define __OUTPUT_LOG__(stream, val) \
do{\
	if (LogStreamWrapper::Get().GetLevel() > m_level){return stream;}\
	tm temp;\
	time_t now = time(NULL);\
	localtime_s(&temp, &now);\
	char strtime[256] = { 0 };\
	size_t rv = strftime(strtime, 256, "%H:%M:%S", &temp);\
	switch (m_level)\
	{\
	case Debug: return (stream << "[" << strtime << "]" << "[DEBUG] " << val).flush();\
	case Info:  return (stream << "[" << strtime << "]" << "[INFO]  " << val).flush();\
	case Error: return (stream << "[" << strtime << "]" << "[ERROR] " << val).flush();\
	}\
	return (stream << "[" << strtime << "]" << "[INFO]  " << val).flush();\
} while (0)

enum LogLevel
{
	Debug = 0,
	Info,
	Error
};

class LogStream;
class LogStreamWrapper
{
	LogStreamWrapper();
	LogStreamWrapper(const LogStreamWrapper&);
	LogStreamWrapper& operator= (const LogStreamWrapper&);
public:
	static LogStreamWrapper& Get();
	LogStream GetStream(LogLevel level);
	LogLevel GetLevel() const;
	LogStreamWrapper& SetLevel(LogLevel level);
	LogStreamWrapper& SetPath(const std::string& path);
	~LogStreamWrapper();
private:
	LogLevel m_level;
	std::ofstream m_fout;
};

class LogStream
{
public:
	LogStream(LogLevel level, std::ostream& stream);
	LogStream(const LogStream& that);
	~LogStream();

	template<typename T>
	std::ostream& operator<< (T val)
	{
		__OUTPUT_LOG__(m_stream, val);
	}

protected:
	LogLevel m_level;
	std::ostream& m_stream;
};

