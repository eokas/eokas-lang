
#ifndef  _EOKAS_BASE_TIME_H_
#define  _EOKAS_BASE_TIME_H_

#include "header.h"
#include "string.h"
#include <ctime>

_BeginNamespace(eokas)
/*
============================================================
== TimeSpan
============================================================
*/
class TimeSpan
{
public:
	static const i64_t US_COUNT_PER_MS;
	static const i64_t US_COUNT_PER_SECOND;
	static const i64_t US_COUNT_PER_MINUTE;
	static const i64_t US_COUNT_PER_HOUR;
	static const i64_t US_COUNT_PER_DAY;

public:
	static TimeSpan maxValue();
	static TimeSpan minValue();

public:
	TimeSpan(i64_t usecs = 0);
	TimeSpan(i32_t days, i32_t hours, i32_t mins, i32_t secs, i32_t msecs = 0, i32_t usecs = 0);
	TimeSpan(const TimeSpan& other);
	~TimeSpan();

public:
	TimeSpan& operator=(const TimeSpan& other);
	TimeSpan operator+(const TimeSpan& other);
	TimeSpan operator-(const TimeSpan& other);
	TimeSpan& operator+=(const TimeSpan& other);
	TimeSpan& operator-=(const TimeSpan& other);	
	bool operator>(const TimeSpan& other);
	bool operator<(const TimeSpan& other);
	bool operator>=(const TimeSpan& other);
	bool operator<=(const TimeSpan& other);
	bool operator==(const TimeSpan& other);
	bool operator!=(const TimeSpan& other);

public:
	i64_t dayPart() const;
	i64_t hourPart() const;
	i64_t minutePart() const;
	i64_t secondPart() const;
	i64_t millisecondPart() const;
	i64_t microsecondPart() const;

	i64_t totalDays() const;
	i64_t totalHours() const;
	i64_t totalMinutes() const;
	i64_t totalSeconds() const;
	i64_t totalMilliseconds() const;

	f64_t exactDays() const;
	f64_t exactHours() const;
	f64_t exactMinutes() const;
	f64_t exactSeconds() const;
	f64_t exactMilliseconds() const;

private:
	i64_t mSpan;
};
/*
============================================================
== TimePoint
============================================================
*/
class TimePoint
{
public:
	TimePoint();
	TimePoint(time_t timeStamp);
	TimePoint(int year, int month, int date, int hour = 0, int minute = 0, int sec = 0);
	TimePoint(const TimePoint& other);
	~TimePoint();

public:
	TimePoint& operator=(const TimePoint& other);
	TimePoint operator+(const TimeSpan& span);
	TimePoint operator-(const TimeSpan& span);
	TimePoint& operator+=(const TimeSpan& span);
	TimePoint& operator-=(const TimeSpan& span);
	TimeSpan operator-(const TimePoint& other);
	bool operator==(const TimePoint& other);
	bool operator!=(const TimePoint& other);
	bool operator>(const TimePoint& other);
	bool operator<(const TimePoint& other);
	bool operator>=(const TimePoint& other);
	bool operator<=(const TimePoint& other);

public:
	u64_t timestamp() const;
	i32_t year() const;
	i32_t month() const;
	i32_t date() const;
	i32_t hour() const;
	i32_t minute() const;
	i32_t second() const;
	i32_t dayOfWeek() const;
	i32_t dayofYear() const;
	bool isLeapYear() const;
	String toString(const String& fmt = "YYYY-MM-DD hh:mm:ss");
	
private:
	time_t mTimeStamp;
	tm* mTimeStruct;
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_TIME_H_
