#include "pch.h"
#include "LogStream.h"
#include <ctime>

LogStreamManager::LogStreamManager() :
	m_level(Debug),
	m_fout("test_log.log", std::ios::out | std::ios::binary | std::ios::app)
{
}

LogStreamManager & LogStreamManager::Get()
{
	static LogStreamManager lm;
	return lm;
}

LogStream LogStreamManager::GetStream(LogLevel level)
{
	return LogStream(level, m_fout);
}

LogLevel LogStreamManager::GetLevel() const
{
	return m_level;
}

LogStreamManager & LogStreamManager::SetLevel(LogLevel level)
{
	m_level = level;
	return *this;
}

LogStreamManager::~LogStreamManager()
{
	if (m_fout)
	{
		m_fout.close();
	}
}

LogStream::LogStream(LogLevel level, std::ostream& stream) :
	m_level(level),
	m_stream(stream)
{
}

LogStream::~LogStream()
{
	if (LogStreamManager::Get().GetLevel() <= m_level)
	{
		(m_stream << std::endl).flush();
	}
}
