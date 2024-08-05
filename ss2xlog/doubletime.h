#ifndef DOUBLETIME_H
#define DOUBLETIME_H

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <format>

#include <cstdint>
#include <ctime>
#include <cmath>
#include <cstring>

class doubletime {
	
	std::string iso8601_utility(bool a_islocal, unsigned int a_trim);
	void eat(const doubletime& a_doubletime); // guts of copy constructor/assignment operator
	
public:

	doubletime();
	doubletime(const double a_double);
	doubletime(const long double a_long_double);
	doubletime(const doubletime& a_doubletime);
	doubletime(doubletime&& a_doubletime);
	~doubletime();
	
	void now();
	bool yet(long double a_increment); // have we reached now + time in seconds yet?
	void set_time(unsigned int a_year, unsigned int a_month, unsigned int a_day, unsigned int a_hour, unsigned int a_minute, unsigned int a_second);
	void set_time_epoch_seconds(std::int64_t a_epoch);
	void set_time_doubletime(double a_time);
	void set_time_long_doubletime(long double a_time);
	
	std::string iso8601_ms();
	std::string iso8601_us();
	std::string iso8601_ns();
	std::string iso8601_ms_zulu();
	std::string iso8601_us_zulu();
	std::string iso8601_ns_zulu();
	std::uint64_t epoch_seconds() const { return m_sec; }
	
	operator long int() const { return m_sec; }
	operator double() const { return (double)m_time; }
	operator long double() const { return m_time; }
	doubletime& operator=(const doubletime& a_doubletime);
	doubletime& operator=(doubletime&& a_doubletime);
	doubletime& operator=(const double a_double);
	doubletime& operator=(const long double a_long_double);
	
	// static helper methods
	static std::string now_as_iso8601_ms();
	static std::string now_as_iso8601_us();
	static std::string now_as_iso8601_ns();
	static std::string now_as_iso8601_ms_zulu();
	static std::string now_as_iso8601_us_zulu();
	static std::string now_as_iso8601_ns_zulu();
	static std::uint64_t now_as_epoch_seconds();
	static double now_as_double();
	static long double now_as_long_double();

protected:
	std::chrono::system_clock::time_point m_tp;
	std::chrono::nanoseconds m_epoch;
	std::int64_t m_sec;
	std::uint64_t m_ns;
	long double m_time;
};

#endif // DOUBLETIME_H
