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

#define UPDATE_TICKS(ticks) \
do\
{\
	ticks = ParseTicks(m_year, m_month, m_day, m_hour, m_minute, m_second, m_millisecond);\
} while (0)



time_t ParseTicks(int year, int month, int day, int hour, int minute, int second, int millisecond) { return 0; }


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
		m_millisecond = that.m_millisecond;
		m_dayOfWeek = that.m_dayOfWeek;
		m_kind = that.m_kind;
	}
	return *this;
}

DateTime & DateTime::operator+(const DateTime & that)
{
	m_year += that.m_year;
	m_month += that.m_month;
	m_day += that.m_day;
	m_hour += that.m_hour;
	m_minute += that.m_minute;
	m_second += that.m_second;
	m_millisecond += that.m_millisecond;

	return *this;
}

DateTime & DateTime::operator-(const DateTime & that)
{
	m_year -= that.m_year;
	m_month -= that.m_month;
	m_day -= that.m_day;
	m_hour -= that.m_hour;
	m_minute -= that.m_minute;
	m_second -= that.m_second;
	m_millisecond -= that.m_millisecond;

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
	return *this;
}

DateTime & DateTime::AddYears(int value)
{
	m_year += value;
	UPDATE_TICKS(m_ticks);
	return *this;
}

DateTime & DateTime::AddMonths(int value)
{
	m_month += value;
	UPDATE_TICKS(m_ticks);
	return *this;
}

DateTime & DateTime::AddDays(double value)
{
	m_ticks += value * SECONDS_PER_DAY;
	UPDATE_TICKS(m_ticks);
	return *this;
}

DateTime & DateTime::AddHours(double value)
{
	m_ticks += value * SECONDS_PER_HOUR;
	UPDATE_TICKS(m_ticks);
	return *this;
}

DateTime & DateTime::AddMinutes(double value)
{
	m_ticks += value * SECONDS_PER_MINUTE;
	UPDATE_TICKS(m_ticks);
	return *this;
}

DateTime & DateTime::AddSeconds(double value)
{
	int second = value;
	int millisecond = (value - second) * 1000;
	m_second += second;
	m_millisecond += millisecond;
	UPDATE_TICKS(m_ticks);
	return *this;
}

DateTime & DateTime::AddMilliseconds(double value)
{
	m_millisecond += value;
	if (m_millisecond >= 1000)
	{
		AddSeconds(m_millisecond / 1000);
		m_millisecond = m_millisecond % 1000;
	}
	UPDATE_TICKS(m_ticks);
	return *this;
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
