#pragma once
#include <ctime>
#include <string>

class DateTime;
class TimeSpan;

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

class TimeSpan
{
	friend class DateTime;
	TimeSpan();
public:
	static TimeSpan FromDays(double value);
	static TimeSpan FromHours(double value);
	static TimeSpan FromMilliseconds(double value);
	static TimeSpan FromMinutes(double value);
	static TimeSpan FromSeconds(double value);
	static TimeSpan FromTicks(time_t value);
public:
	TimeSpan(time_t ticks);
	TimeSpan(int hours, int minutes, int seconds);
	TimeSpan(int days, int hours, int minutes, int seconds);
	TimeSpan(int days, int hours, int minutes, int seconds, int milliseconds);
	TimeSpan(const TimeSpan& that);
	TimeSpan& operator= (const TimeSpan& that);
	TimeSpan  operator+ (const TimeSpan& that) const;
	TimeSpan  operator- (const TimeSpan& that) const;

	bool operator>  (const TimeSpan& that) const;
	bool operator<  (const TimeSpan& that) const;
	bool operator>= (const TimeSpan& that) const;
	bool operator<= (const TimeSpan& that) const;
	bool operator== (const TimeSpan& that) const;
	bool operator!= (const TimeSpan& that) const;

	TimeSpan& Add(const TimeSpan& timespan);
	TimeSpan& Subtract(const TimeSpan& timespan);
	TimeSpan  Duration() const;
	TimeSpan  Negate() const;

	~TimeSpan();

private:
	void* m_data;

public:
	static const TimeSpan& const MaxValue;
	static const TimeSpan& const MinValue;
public:
	const time_t& const Ticks;
	const double& const TotalDays;
	const double& const TotalHours;
	const double& const TotalMinutes;
	const double& const TotalSeconds;
	const double& const TotalMilliseconds;
	const int& const Days;
	const int& const Hours;
	const int& const Minutes;
	const int& const Seconds;
	const int& const Milliseconds;
};

class DateTime
{
	friend class TimeSpan;
	DateTime();
public:
	static DateTime Now();
	static DateTime Today();
	static DateTime UtcNow();
public:
	DateTime(time_t unixTimeStamp);
	DateTime(time_t unixTimeStamp, DateTimeKind kind);
	DateTime(int year, int month, int day);
	DateTime(int year, int month, int day, int hour, int minute, int second);
	DateTime(int year, int month, int day, int hour, int minute, int second, DateTimeKind kind);
	DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond);
	DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond, DateTimeKind kind);
	DateTime(const DateTime& that);
	DateTime& operator= (const DateTime& that);
	DateTime& operator+ (const TimeSpan& that);
	DateTime& operator- (const TimeSpan& that);
	TimeSpan  operator- (const DateTime& that);
	bool operator<  (const DateTime& that) const;
	bool operator>  (const DateTime& that) const;
	bool operator<= (const DateTime& that) const;
	bool operator>= (const DateTime& that) const;
	bool operator== (const DateTime& that) const;
	bool operator!= (const DateTime& that) const;

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

private:
	void* m_data;

public:
	const time_t& const UnixTimeStamp;
	const int& const Year;
	const int& const Month;
	const int& const Day;
	const int& const Hour;
	const int& const Minute;
	const int& const Second;
	const int& const Millisecond;
	const enum DayOfWeek& const DayOfWeek;
	const enum DateTimeKind& const Kind;
};

