#pragma once
#include <ctime>
#include <string>

enum DateTimeKind
{
	//表示的时间既未指定为本地时间，也未指定为协调通用时间 (UTC)。
	Unspecified = 0,
	//表示的时间为 UTC。
	Utc = 1,
	//表示的时间为本地时间。
	Local = 2
};

enum DayOfWeek
{
	//星期日
	Sunday = 0,
	//星期一
	Monday = 1,
	//星期二
	Tuesday = 2,
	//星期三
	Wednesday = 3,
	//星期四
	Thursday = 4,
	//星期五
	Friday = 5,
	//星期六
	Saturday = 6
};

class DateTime
{
	DateTime();
public:
	static DateTime Now();
	static DateTime Today();
	static DateTime UtcNow();
public:
	DateTime(time_t ticks);
	DateTime(time_t ticks, DateTimeKind kind);
	DateTime(int year, int month, int day);
	DateTime(int year, int month, int day, int hour, int minute, int second);
	DateTime(int year, int month, int day, int hour, int minute, int second, DateTimeKind kind);
	DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond);
	DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond, DateTimeKind kind);
	DateTime(const DateTime& that);
	DateTime& operator= (const DateTime& that);
	DateTime& operator+ (const DateTime& that);
	DateTime& operator- (const DateTime& that);
	bool operator<  (const DateTime& that);
	bool operator>  (const DateTime& that);
	bool operator<= (const DateTime& that);
	bool operator>= (const DateTime& that);
	bool operator== (const DateTime& that);
	bool operator!= (const DateTime& that);

	DateTime& AddTicks(time_t value);
	DateTime& AddYears(int value);
	DateTime& AddMonths(int value);
	DateTime& AddDays(double value);
	DateTime& AddHours(double value);
	DateTime& AddMinutes(double value);
	DateTime& AddSeconds(double value);
	DateTime& AddMilliseconds(double value);
	DateTime ToLocalTime() const;
	DateTime ToUniversalTime() const;
	std::string ToLongDateString() const;
	std::string ToLongTimeString() const;
	std::string ToShortDateString() const;
	std::string ToShortTimeString() const;
	std::string ToString(const std::string& format) const;

	~DateTime();

public:
	const time_t& Ticks;
	const int& Year;
	const int& Month;
	const int& Day;
	const int& Hour;
	const int& Minute;
	const int& Second;
	const int& Millisecond;
	const enum DayOfWeek& DayOfWeek;
	const enum DateTimeKind& Kind;

private:
	time_t m_ticks;
	int m_year;
	int m_month;
	int m_day;
	int m_hour;
	int m_minute;
	int m_second;
	int m_millisecond;
	enum DayOfWeek m_dayOfWeek;
	DateTimeKind m_kind;
};

