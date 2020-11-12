#pragma once
#include <ctime>
#include <string>
#pragma warning(disable:4227)
#pragma warning(disable:4244)
class DateTime;
class TimeSpan;

enum DateTimeKind
{
	//��ʾ��ʱ���δָ��Ϊ����ʱ�䣬Ҳδָ��ΪЭ��ͨ��ʱ�� (UTC)��
	Unspecified = 0,
	//��ʾ��ʱ��Ϊ UTC��
	Utc = 1,
	//��ʾ��ʱ��Ϊ����ʱ�䡣
	Local = 2
};

enum DayOfWeek
{
	//������
	Sunday = 0,
	//����һ
	Monday = 1,
	//���ڶ�
	Tuesday = 2,
	//������
	Wednesday = 3,
	//������
	Thursday = 4,
	//������
	Friday = 5,
	//������
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
	static const TimeSpan& MaxValue;
	static const TimeSpan& MinValue;
public:
	const time_t& Ticks;
	const double& TotalDays;
	const double& TotalHours;
	const double& TotalMinutes;
	const double& TotalSeconds;
	const double& TotalMilliseconds;
	const int& Days;
	const int& Hours;
	const int& Minutes;
	const int& Seconds;
	const int& Milliseconds;
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
	const time_t& UnixTimeStamp;
	const int& Year;
	const int& Month;
	const int& Day;
	const int& Hour;
	const int& Minute;
	const int& Second;
	const int& Millisecond;
	const enum DayOfWeek& DayOfWeek;
	const enum DateTimeKind& Kind;
};

