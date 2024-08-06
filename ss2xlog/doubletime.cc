#include "doubletime.h"

namespace ss {

doubletime::doubletime()
{
	now();
}

doubletime::doubletime(const double a_double)
{
	set_time_doubletime(a_double);
}

doubletime::doubletime(const long double a_long_double)
{
	set_time_long_doubletime(a_long_double);
}

doubletime::doubletime(const std::int64_t a_epoch)
{
	set_time_epoch_seconds(a_epoch);
}

void doubletime::eat(const doubletime& a_doubletime)
{
	m_tp = a_doubletime.m_tp;
	m_epoch = a_doubletime.m_epoch;
	m_sec = a_doubletime.m_sec;
	m_ns = a_doubletime.m_ns;
	m_time = a_doubletime.m_time;
}

doubletime::doubletime(const doubletime& a_doubletime)
{
	eat(a_doubletime);
}

doubletime::doubletime(doubletime&& a_doubletime)
{
	eat(a_doubletime);
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

bool doubletime::yet(long double a_increment)
{
	doubletime l_now;
	return (l_now.m_time >= (m_time + a_increment));
}

void doubletime::set_time(unsigned int a_year, unsigned int a_month, unsigned int a_day, unsigned int a_hour, unsigned int a_minute, unsigned int a_second)
{
	struct tm t;
	memset(&t, 0, sizeof(struct tm));
	t.tm_year = a_year - 1900;
	t.tm_mon = a_month - 1;
	t.tm_mday = a_day;
	t.tm_hour = a_hour;
	t.tm_min = a_minute;
	t.tm_sec = a_second;
	time_t a_epoch_time = mktime(&t);
	m_tp = std::chrono::system_clock::from_time_t(a_epoch_time);
	m_epoch = m_tp.time_since_epoch();
	m_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(m_epoch).count() % 1000000000;
	m_sec = std::chrono::duration_cast<std::chrono::seconds>(m_epoch).count();
	m_time = (long double)m_sec + ((long double)m_ns / 1000000000.0L);
}

void doubletime::set_time_epoch_seconds(std::int64_t a_epoch)
{
	time_t a_epoch_time = a_epoch;
	m_tp = std::chrono::system_clock::from_time_t(a_epoch_time);
	m_epoch = m_tp.time_since_epoch();
	m_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(m_epoch).count() % 1000000000;
	m_sec = std::chrono::duration_cast<std::chrono::seconds>(m_epoch).count();
	m_time = (long double)m_sec + ((long double)m_ns / 1000000000.0L);
}

void doubletime::set_time_doubletime(double a_time)
{
	set_time_long_doubletime((long double)a_time);
}

void doubletime::set_time_long_doubletime(long double a_time)
{
	long double l_int;
	long double l_frac = std::modfl(a_time, &l_int);
	unsigned int l_nsec = (unsigned int)(l_frac * 1000000000.0);
	unsigned int l_sec = (unsigned int)l_int;
	auto l_duration = std::chrono::seconds { l_sec } + std::chrono::nanoseconds { l_nsec };
	std::chrono::nanoseconds l_ndur = std::chrono::duration_cast<std::chrono::nanoseconds>(l_duration);
	auto l_ndur_tp = std::chrono::system_clock::time_point(l_ndur);
	m_tp = l_ndur_tp;
	m_epoch = m_tp.time_since_epoch();
	m_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(m_epoch).count() % 1000000000;
	m_sec = std::chrono::duration_cast<std::chrono::seconds>(m_epoch).count();
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

doubletime& doubletime::operator=(const doubletime& a_doubletime)
{
	eat(a_doubletime);
	return *this;
}

doubletime& doubletime::operator=(doubletime&& a_doubletime)
{
	eat(a_doubletime);
	return *this;
}

doubletime& doubletime::operator=(const double a_double)
{
	set_time_doubletime(a_double);
	return *this;
}

doubletime& doubletime::operator=(const long double a_long_double)
{
	set_time_long_doubletime(a_long_double);
	return *this;
}

doubletime& doubletime::operator=(const std::int64_t a_epoch)
{
	set_time_epoch_seconds(a_epoch);
	return *this;
}

// breakouts

void doubletime::get_tm(bool a_islocal)
{
	time_t l_tt = std::chrono::system_clock::to_time_t(m_tp);
	if (a_islocal)
		m_tm = *localtime(&l_tt);
	else
		m_tm = *gmtime(&l_tt);
}

unsigned int doubletime::zulu_year()
{
	get_tm(false);
	return m_tm.tm_year + 1900;
}

unsigned int doubletime::local_year()
{
	get_tm(true);
	return m_tm.tm_year + 1900;
}

unsigned int doubletime::zulu_month()
{
	get_tm(false);
	return m_tm.tm_mon + 1;
}

unsigned int doubletime::local_month()
{
	get_tm(true);
	return m_tm.tm_mon + 1;
}

unsigned int doubletime::zulu_day()
{
	get_tm(false);
	return m_tm.tm_mday;
}

unsigned int doubletime::local_day()
{
	get_tm(true);
	return m_tm.tm_mday;
}

unsigned int doubletime::zulu_day_of_week()
{
	get_tm(false);
	return m_tm.tm_wday;
}

unsigned int doubletime::local_day_of_week()
{
	get_tm(true);
	return m_tm.tm_wday;
}

unsigned int doubletime::zulu_hour()
{
	get_tm(false);
	return m_tm.tm_hour;
}

unsigned int doubletime::local_hour()
{
	get_tm(true);
	return m_tm.tm_hour;
}

unsigned int doubletime::zulu_minute()
{
	get_tm(false);
	return m_tm.tm_min;
}

unsigned int doubletime::local_minute()
{
	get_tm(true);
	return m_tm.tm_min;
}

unsigned int doubletime::zulu_second()
{
	get_tm(false);
	return m_tm.tm_sec;
}

unsigned int doubletime::local_second()
{
	get_tm(true);
	return m_tm.tm_sec;
}

unsigned int doubletime::zulu_day_of_year()
{
	get_tm(false);
	return m_tm.tm_yday;
}

unsigned int doubletime::local_day_of_year()
{
	get_tm(true);
	return m_tm.tm_yday;
}

bool doubletime::is_dst()
{
	get_tm(true);
	return (m_tm.tm_isdst > 0);
}

// timestamps

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

std::int64_t doubletime::now_as_epoch_seconds()
{
	doubletime l_now;
	return l_now.epoch_seconds();
}

double doubletime::now_as_double()
{
	doubletime l_now;
	return (double)l_now;
}

long double doubletime::now_as_long_double()
{
	doubletime l_now;
	return (long double)l_now;
}

} // namespace ss
