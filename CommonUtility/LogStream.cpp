#include "pch.h"
#include "LogStream.h"
#include <ctime>

LogStreamWrapper::LogStreamWrapper() :
	m_level(Debug)
{
}

LogStreamWrapper::LogStreamWrapper(const LogStreamWrapper &)
{
}

LogStreamWrapper & LogStreamWrapper::operator=(const LogStreamWrapper &)
{
	return *this;
}

LogStreamWrapper & LogStreamWrapper::Get()
{
	static LogStreamWrapper lm;
	return lm;
}

LogStream LogStreamWrapper::GetStream(LogLevel level)
{
	if (m_fout.is_open())
	{
		return LogStream(level, m_fout);
	}
	return LogStream(level, std::cout);
}

LogLevel LogStreamWrapper::GetLevel() const
{
	return m_level;
}

LogStreamWrapper & LogStreamWrapper::SetLevel(LogLevel level)
{
	m_level = level;
	return *this;
}

LogStreamWrapper & LogStreamWrapper::SetPath(const std::string & path)
{
	if (!path.empty())
	{
		if (m_fout)
		{
			m_fout.close();
		}
		m_fout.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::app);
	}
	return *this;
}

LogStreamWrapper::~LogStreamWrapper()
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

LogStream::LogStream(const LogStream & that) :
	m_level(that.m_level),
	m_stream(that.m_stream)
{
}

LogStream::~LogStream()
{
	if (LogStreamWrapper::Get().GetLevel() <= m_level)
	{
		(m_stream << std::endl).flush();
	}
}