#include "pch.h"
#include "DateTime.h"

#define NOW time(NULL)

#define SECONDS_PER_MINUTE (60)
#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * 60)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * 24)

#define INIT_CONST_REFERENCES \
Ticks(m_ticks),\
Year(m_year),\
Month(m_month),\
Day(m_day),\
Hour(m_hour),\
Minute(m_minute),\
Second(m_second),\
Millisecond(m_millisecond),\
DayOfWeek(m_dayOfWeek),\
Kind(m_kind)\

time_t ParseTicksFromDateTime(
	int year, int month, int day,
	int hour, int minute, int second,
	enum DayOfWeek& wday)
{
	tm temp;
	temp.tm_year = year - 1900;
	temp.tm_mon = month - 1;
	temp.tm_mday = day;
	temp.tm_hour = hour;
	temp.tm_min = minute;
	temp.tm_sec = second;

	//处理星期几
	time_t ticks = mktime(&temp);
	localtime_s(&temp, &ticks);
	wday = static_cast<enum DayOfWeek>(temp.tm_wday);

	return ticks;
}

void ParseDateTimeFromTicks(
	DateTimeKind kind,
	time_t ticks,
	int& year, int& month, int& day,
	int& hour, int& minute, int& second,
	enum DayOfWeek& wday)
{
	tm temp;

	switch (kind)
	{
	case Utc:
		gmtime_s(&temp, &ticks);
		break;
	case Local:
		localtime_s(&temp, &ticks);
		break;
	default:
		localtime_s(&temp, &ticks);
		break;
	}

	year = temp.tm_year + 1900;
	month = temp.tm_mon + 1;
	day = temp.tm_mday;
	hour = temp.tm_hour;
	minute = temp.tm_min;
	second = temp.tm_sec;
	wday = static_cast<enum DayOfWeek>(temp.tm_wday);
}


DateTime::DateTime() :
	m_ticks(NOW),
	m_year(0),
	m_month(0),
	m_day(0),
	m_hour(0),
	m_minute(0),
	m_second(0),
	m_millisecond(0),
	m_dayOfWeek(DayOfWeek::Sunday),
	m_kind(DateTimeKind::Unspecified),
	INIT_CONST_REFERENCES
{
	ParseDateTimeFromTicks(
		m_kind,
		m_ticks,
		m_year, m_month, m_day,
		m_hour, m_minute, m_second,
		m_dayOfWeek);
}


DateTime::~DateTime()
{
}

DateTime::DateTime(time_t ticks) :
	INIT_CONST_REFERENCES,
	m_ticks(ticks),
	m_year(0),
	m_month(0),
	m_day(0),
	m_hour(0),
	m_minute(0),
	m_second(0),
	m_millisecond(0),
	m_dayOfWeek(DayOfWeek::Sunday),
	m_kind(DateTimeKind::Unspecified)
{
	ParseDateTimeFromTicks(
		m_kind,
		m_ticks,
		m_year, m_month, m_day,
		m_hour, m_minute, m_second,
		m_dayOfWeek);
}

DateTime::DateTime(time_t ticks, DateTimeKind kind) :
	INIT_CONST_REFERENCES,
	m_ticks(ticks),
	m_year(0),
	m_month(0),
	m_day(0),
	m_hour(0),
	m_minute(0),
	m_second(0),
	m_millisecond(0),
	m_dayOfWeek(DayOfWeek::Sunday),
	m_kind(kind)
{
	ParseDateTimeFromTicks(
		m_kind,
		m_ticks,
		m_year, m_month, m_day,
		m_hour, m_minute, m_second,
		m_dayOfWeek);
}

DateTime::DateTime(int year, int month, int day) :
	INIT_CONST_REFERENCES,
	m_ticks(0),
	m_year(year),
	m_month(month),
	m_day(day),
	m_hour(0),
	m_minute(0),
	m_second(0),
	m_millisecond(0),
	m_dayOfWeek(DayOfWeek::Sunday),
	m_kind(DateTimeKind::Unspecified)
{
	m_ticks = ParseTicksFromDateTime(m_year, m_month, m_day, m_hour, m_minute, m_second, m_dayOfWeek);
}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second) :
	INIT_CONST_REFERENCES,
	m_ticks(0),
	m_year(year),
	m_month(month),
	m_day(day),
	m_hour(hour),
	m_minute(minute),
	m_second(second),
	m_millisecond(0),
	m_dayOfWeek(DayOfWeek::Sunday),
	m_kind(DateTimeKind::Unspecified)
{
	m_ticks = ParseTicksFromDateTime(m_year, m_month, m_day, m_hour, m_minute, m_second, m_dayOfWeek);
}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, DateTimeKind kind) :
	INIT_CONST_REFERENCES,
	m_ticks(0),
	m_year(year),
	m_month(month),
	m_day(day),
	m_hour(hour),
	m_minute(minute),
	m_second(second),
	m_millisecond(0),
	m_dayOfWeek(DayOfWeek::Sunday),
	m_kind(kind)
{
}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond) :
	INIT_CONST_REFERENCES,
	m_ticks(0),
	m_year(year),
	m_month(month),
	m_day(day),
	m_hour(hour),
	m_minute(minute),
	m_second(second),
	m_millisecond(millisecond),
	m_dayOfWeek(DayOfWeek::Sunday),
	m_kind(DateTimeKind::Unspecified)
{
	m_ticks = ParseTicksFromDateTime(m_year, m_month, m_day, m_hour, m_minute, m_second, m_dayOfWeek);
}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond, DateTimeKind kind) :
	INIT_CONST_REFERENCES,
	m_ticks(0),
	m_year(year),
	m_month(month),
	m_day(day),
	m_hour(hour),
	m_minute(minute),
	m_second(second),
	m_millisecond(millisecond),
	m_dayOfWeek(DayOfWeek::Sunday),
	m_kind(kind)
{
	m_ticks = ParseTicksFromDateTime(m_year, m_month, m_day, m_hour, m_minute, m_second, m_dayOfWeek);
}


DateTime::DateTime(const DateTime & that) :
	INIT_CONST_REFERENCES,
	m_ticks(that.m_ticks),
	m_year(that.m_year),
	m_month(that.m_month),
	m_day(that.m_day),
	m_hour(that.m_hour),
	m_minute(that.m_minute),
	m_second(that.m_second),
	m_millisecond(that.m_millisecond),
	m_dayOfWeek(that.m_dayOfWeek),
	m_kind(that.m_kind)
{
}

DateTime & DateTime::operator=(const DateTime & that)
{
	if (&that != this)
	{
		m_year = that.m_year;
		m_month = that.m_month;
		m_day = that.m_day;
		m_hour = that.m_hour;
		m_minute = that.m_minute;
		m_second = that.m_second;
		m_dayOfWeek = that.m_dayOfWeek;
		m_ticks = that.m_ticks;
		m_millisecond = that.m_millisecond;
		m_kind = that.m_kind;
	}
	return *this;
}

DateTime & DateTime::operator+(const DateTime & that)
{
	m_ticks += that.m_ticks;

	if (m_ticks < 0)
	{
		m_ticks = 0;
	}

	ParseDateTimeFromTicks(
		m_kind,
		m_ticks,
		m_year, m_month, m_day,
		m_hour, m_minute, m_second,
		m_dayOfWeek);

	return *this;
}

DateTime & DateTime::operator-(const DateTime & that)
{
	m_ticks -= that.m_ticks;

	if (m_ticks < 0)
	{
		m_ticks = 0;
	}

	ParseDateTimeFromTicks(
		m_kind,
		m_ticks,
		m_year, m_month, m_day,
		m_hour, m_minute, m_second,
		m_dayOfWeek);

	return *this;
}

bool DateTime::operator<(const DateTime & that)
{
	return m_ticks < that.m_ticks;
}

bool DateTime::operator>(const DateTime & that)
{
	return m_ticks > that.m_ticks;
}

bool DateTime::operator<=(const DateTime & that)
{
	return m_ticks <= that.m_ticks;
}

bool DateTime::operator>=(const DateTime & that)
{
	return m_ticks >= that.m_ticks;
}

bool DateTime::operator==(const DateTime & that)
{
	return m_ticks == that.m_ticks;
}

bool DateTime::operator!=(const DateTime & that)
{
	return m_ticks != that.m_ticks;
}

DateTime & DateTime::AddTicks(time_t value)
{
	m_ticks += value;
	if (m_ticks < 0)
	{
		m_ticks = 0;
	}

	ParseDateTimeFromTicks(
		m_kind,
		m_ticks,
		m_year, m_month, m_day,
		m_hour, m_minute, m_second,
		m_dayOfWeek);

	return *this;
}

DateTime & DateTime::AddYears(int value)
{
	m_year += value;
	m_ticks = ParseTicksFromDateTime(
		m_year, m_month, m_day,
		m_hour, m_minute, m_second,
		m_dayOfWeek);
	return *this;
}

DateTime & DateTime::AddMonths(int value)
{
	m_month += value;
	m_ticks = ParseTicksFromDateTime(
		m_year, m_month, m_day,
		m_hour, m_minute, m_second,
		m_dayOfWeek);
	return *this;
}

DateTime & DateTime::AddDays(double value)
{
	return AddSeconds(value * SECONDS_PER_DAY);
}

DateTime & DateTime::AddHours(double value)
{
	return AddSeconds(value * SECONDS_PER_HOUR);
}

DateTime & DateTime::AddMinutes(double value)
{
	return AddSeconds(value * SECONDS_PER_MINUTE);
}

DateTime & DateTime::AddSeconds(double value)
{
	double seconds = (m_ticks + value);
	m_ticks = seconds;
	m_millisecond = (seconds - m_ticks) * 1000;

	ParseDateTimeFromTicks(
		m_kind,
		m_ticks,
		m_year, m_month, m_day,
		m_hour, m_minute, m_second,
		m_dayOfWeek);

	return *this;
}

DateTime & DateTime::AddMilliseconds(double value)
{
	return AddSeconds(value / 1000);
}

DateTime DateTime::ToLocalTime() const
{
	return DateTime(m_ticks, DateTimeKind::Local);
}

DateTime DateTime::ToUniversalTime() const
{
	return DateTime(m_ticks, DateTimeKind::Utc);
}

std::string DateTime::ToLongDateString() const
{
	char buffer[256] = { 0 };
	sprintf_s(buffer, "%d年%d月%d日", m_year, m_month, m_day);
	return buffer;
}

std::string DateTime::ToLongTimeString() const
{
	char buffer[256] = { 0 };
	sprintf_s(buffer, "%d:%d:%d", m_hour, m_minute, m_second);
	return buffer;
}

std::string DateTime::ToShortDateString() const
{
	char buffer[256] = { 0 };
	sprintf_s(buffer, "%d/%d/%d", m_year, m_month, m_day);
	return buffer;
}

std::string DateTime::ToShortTimeString() const
{
	char buffer[256] = { 0 };
	sprintf_s(buffer, "%d:%d", m_hour, m_minute);
	return buffer;
}

std::string DateTime::ToString(const std::string & format) const
{
	if (format.empty())
	{
		return "";
	}

	char buffer[256] = { 0 };
	tm temp;
	switch (m_kind)
	{
	case Utc:
		gmtime_s(&temp, &m_ticks);
		break;
	case Local:
		localtime_s(&temp, &m_ticks);
		break;
	default:
		localtime_s(&temp, &m_ticks);
		break;
	}
	size_t rv = strftime(buffer, 256, format.c_str(), &temp);
	return std::string(buffer, rv);
}

DateTime DateTime::Now()
{
	return DateTime(NOW);
}

DateTime DateTime::Today()
{
	DateTime now(NOW);
	return DateTime(now.Year, now.Month, now.Day);
}

DateTime DateTime::UtcNow()
{
	return DateTime(NOW, DateTimeKind::Utc);
}
