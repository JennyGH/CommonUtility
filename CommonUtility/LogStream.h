#pragma once
#include <fstream>
#include <ctime>

#define LOG_DEBUG (LogStreamManager::Get().GetStream(Debug))
#define LOG_INFO  (LogStreamManager::Get().GetStream(Info))
#define LOG_ERROR (LogStreamManager::Get().GetStream(Error))

#define __OUTPUT_LOG__(stream, val) \
do{\
	if (LogStreamManager::Get().GetLevel() > m_level){return stream;}\
	switch (m_level)\
	{\
	case Debug: return (stream << "[DEBUG] " << val).flush();\
	case Info:  return (stream << "[INFO]  " << val).flush();\
	case Error: return (stream << "[ERROR] " << val).flush();\
	}\
	return (stream << "[INFO]  " << val).flush();\
} while (0)

enum LogLevel
{
	Debug = 0,
	Info,
	Error
};

class LogStream;
class LogStreamManager
{
	LogStreamManager();
public:
	static LogStreamManager& Get();
	LogStream GetStream(LogLevel level);
	LogLevel GetLevel() const;
	LogStreamManager& SetLevel(LogLevel level);
	~LogStreamManager();
private:
	LogLevel m_level;
	std::ofstream m_fout;
};

class LogStream
{
public:
	LogStream(LogLevel level, std::ostream& stream);
	~LogStream();

	template<typename T>
	std::ostream& operator<< (T val)
	{
		auto now = time(NULL);
		tm temp;
		localtime_s(&temp, &now);
		{
			tm* ptm = NULL;
			ptm = localtime(&now);
			temp.tm_year = ptm->tm_year;
			temp.tm_mon = ptm->tm_mon;
			temp.tm_mday = ptm->tm_mday;
			temp.tm_wday = ptm->tm_wday;
			temp.tm_yday = ptm->tm_yday;
			temp.tm_hour = ptm->tm_hour;
			temp.tm_min = ptm->tm_min;
			temp.tm_sec = ptm->tm_sec;
			temp.tm_isdst = ptm->tm_isdst;
		}
		temp = *localtime(&now);
		char buffer[256] = { 0 };
		size_t rv = strftime(buffer, 256, "%H:%M:%S", &temp);
		__OUTPUT_LOG__(m_stream, val);
	}

protected:
	LogLevel m_level;
	std::ostream& m_stream;
};

