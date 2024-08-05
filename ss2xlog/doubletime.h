#ifndef DOUBLETIME_H
#define DOUBLETIME_H

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <format>

#include <cstdint>

class doubletime {
	
	std::string iso8601_utility(bool a_islocal, unsigned int a_trim);
	
public:

	doubletime();
	~doubletime();
	
	void now();
	
	std::string iso8601_ms();
	std::string iso8601_us();
	std::string iso8601_ns();
	std::string iso8601_ms_zulu();
	std::string iso8601_us_zulu();
	std::string iso8601_ns_zulu();
	std::uint64_t epoch_seconds() const { return m_sec; }
	
	operator long unsigned int() const { return m_sec; }
	operator double() const { return (double)m_time; }
	operator long double() const { return m_time; }
	
	// static helper methods
	static std::string now_as_iso8601_ms();
	static std::string now_as_iso8601_us();
	static std::string now_as_iso8601_ns();
	static std::string now_as_iso8601_ms_zulu();
	static std::string now_as_iso8601_us_zulu();
	static std::string now_as_iso8601_ns_zulu();
	static std::uint64_t now_as_epoch_seconds();	

protected:
	std::chrono::system_clock::time_point m_tp;
	std::chrono::nanoseconds m_epoch;
	std::uint64_t m_sec;
	std::uint64_t m_ns;
	long double m_time;
};

#endif // DOUBLETIME_H
