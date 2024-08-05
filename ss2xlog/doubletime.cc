#include "doubletime.h"

doubletime::doubletime()
{
	now();
}

doubletime::~doubletime()
{
	
}

void doubletime::now()
{
	m_tp = std::chrono::system_clock::now();
	m_epoch = m_tp.time_since_epoch();
	m_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(m_epoch).count() % 1000000000;
	m_sec = std::chrono::duration_cast<std::chrono::seconds>(m_epoch).count();
//	std::cout << "sec " << l_sec << " ns " << l_ns << std::endl;
	m_time = (long double)m_sec + ((long double)m_ns / 1000000000.0L);
}

std::string doubletime::iso8601_utility(bool a_islocal, unsigned int a_trim)
{
	std::stringstream l_ret;
	std::string l_sec_trim = std::format("{:%S}", m_tp).substr(0, 2);
	std::stringstream l_nano_trim_ss;
	l_nano_trim_ss << "000000000" << m_ns;
	std::string l_nano_trim = l_nano_trim_ss.str();
	l_nano_trim = l_nano_trim.substr(l_nano_trim.size() - 9, 9);
	l_nano_trim = l_nano_trim.substr(0, a_trim);
	auto m_tp_local = std::chrono::zoned_time(std::chrono::current_zone(), m_tp);
	if (a_islocal)
		l_ret << std::format("{0:%Y}-{0:%m}-{0:%d}T{0:%H}:{0:%M}:{1:}", m_tp_local, l_sec_trim) << "," << l_nano_trim << std::format("{0:%z}{0:%Z}", m_tp_local);
	else
		l_ret << std::format("{0:%Y}-{0:%m}-{0:%d}T{0:%H}:{0:%M}:{1:}", m_tp, l_sec_trim) << "," << l_nano_trim << "Z";
	return l_ret.str();
}

std::string doubletime::iso8601_ms()
{
	return iso8601_utility(true, 3);
}

std::string doubletime::iso8601_us()
{
	return iso8601_utility(true, 6);
}

std::string doubletime::iso8601_ns()
{
	return iso8601_utility(true, 9);
}

std::string doubletime::iso8601_ms_zulu()
{
	return iso8601_utility(false, 3);
}

std::string doubletime::iso8601_us_zulu()
{
	return iso8601_utility(false, 6);
}

std::string doubletime::iso8601_ns_zulu()
{
	return iso8601_utility(false, 9);
}

/* statics */

std::string doubletime::now_as_iso8601_ms()
{
	doubletime l_now;
	return l_now.iso8601_ms();
}

std::string doubletime::now_as_iso8601_us()
{
	doubletime l_now;
	return l_now.iso8601_us();
}

std::string doubletime::now_as_iso8601_ns()
{
	doubletime l_now;
	return l_now.iso8601_ns();
}

std::string doubletime::now_as_iso8601_ms_zulu()
{
	doubletime l_now;
	return l_now.iso8601_ms_zulu();
}

std::string doubletime::now_as_iso8601_us_zulu()
{
	doubletime l_now;
	return l_now.iso8601_us_zulu();
}

std::string doubletime::now_as_iso8601_ns_zulu()
{
	doubletime l_now;
	return l_now.iso8601_ns_zulu();
}

std::uint64_t doubletime::now_as_epoch_seconds()
{
	doubletime l_now;
	return l_now.epoch_seconds();
}
