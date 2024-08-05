#include "log.h"

namespace ss {

namespace log {

const std::string prio_str[] = { "EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG" }; // Names of priority values.

// use the routines in doubletime instead
//std::string iso8601()
//{
//	std::stringstream l_ret_ss;
//	std::chrono::high_resolution_clock::time_point l_now = std::chrono::high_resolution_clock::now();
//	uint64_t frac_us = std::chrono::duration_cast<std::chrono::microseconds>(l_now.time_since_epoch()).count() % 1000000;
//	std::time_t l_now_tt = std::chrono::high_resolution_clock::to_time_t(l_now);
//	struct std::tm *l_now_tm = std::localtime(&l_now_tt);
//	l_ret_ss << std::put_time(l_now_tm, "%FT%T") << "," << std::setw(6) << std::setfill('0') << frac_us;
//	return l_ret_ss.str();
//}

// target_base

target_base::target_base(prio_t a_threshold, std::string a_format)
: m_priority(DEBUG)
, m_threshold(a_threshold)
, m_format(a_format)
, m_enable_color(true)
{
	
}

void target_base::set_p(prio_t a_priority)
{
	m_priority = a_priority;
}

void target_base::set_enable_color(bool a_enable)
{
	m_enable_color = a_enable;
}

void target_base::accept_logtext_p(prio_t a_priority, std::string a_line, std::string a_thread_name, const std::source_location& a_location)
{
	prio_t temp = m_priority;
	m_priority = a_priority;
	accept_logtext(a_line, a_thread_name, a_location);
	m_priority = temp;
}

void target_base::accept_logtext(std::string a_line, std::string a_thread_name, const std::source_location& a_location)
{
	// priority filtered?
	if (m_priority > m_threshold)
		return;
		
	// attempt to build a formatted line based on our input
	std::string l_out = m_format;
	size_t found;
	
	found = l_out.find("%%message%%");
	if (found != std::string::npos) {
		l_out.replace(found, 11, a_line);
	}
	found = l_out.find("%%iso8601%%");
	if (found != std::string::npos) {
		l_out.replace(found, 11, doubletime::now_as_iso8601_us());
	}
	found = l_out.find("%%priority%%");
	if (found != std::string::npos) {
		l_out.replace(found, 12, prio_str[m_priority]);
	}
	found = l_out.find("%%file%%");
	if (found != std::string::npos) {
		l_out.replace(found, 8, a_location.file_name());
	}
	found = l_out.find("%%line%%");
	if (found != std::string::npos) {
		l_out.replace(found, 8, std::format("{}", a_location.line()));
	}
	found = l_out.find("%%function%%");
	if (found != std::string::npos) {
		l_out.replace(found, 12, std::format("{}", a_location.function_name()));
	}
	found = l_out.find("%%thread%%");
	if (found != std::string::npos) {
		l_out.replace(found, 10, a_thread_name);
	}
	// imbue color - colors can appear more than once so loop until no more instances of this color are found
	for (auto& [key, value] : color_tokens) {
		do {
			found = l_out.find(key);
			if (found != std::string::npos) {
				if (m_enable_color)
					l_out.replace(found, key.size(), value);
				else
					l_out.replace(found, key.size(), "");
			}
		} while (found != std::string::npos);
	}
	post_logtext(l_out);
}

// stdout

const std::string target_stdout::DEFAULT_FORMATTER = "[%%COLORGREEN%%%%iso8601%%%%COLORDEFAULT%%] [%%COLORLIGHTRED%%%%priority%%%%COLORDEFAULT%%] [%%COLORLIGHTBLUE%%%%thread%%%%COLORDEFAULT%%] %%message%%";
const std::string target_stdout::DEFAULT_FORMATTER_DEBUGINFO = "[%%COLORGREEN%%%%iso8601%%%%COLORDEFAULT%%] [%%COLORLIGHTRED%%%%priority%%%%COLORDEFAULT%%] [%%COLORLIGHTBLUE%%%%thread%%%%COLORDEFAULT%%] [%%COLORCYAN%%%%file%%/%%line%% %%COLORLIGHTYELLOW%%%%function%%%%COLORDEFAULT%%] %%message%%";

target_stdout::target_stdout(prio_t a_threshold, std::string a_format)
: target_base(a_threshold, a_format)
{
	
}

target_stdout::~target_stdout()
{
	
}

void target_stdout::post_logtext(std::string& a_formatted_message)
{
	std::cout << a_formatted_message << std::endl;
}

// file

const std::string target_file::DEFAULT_FORMATTER = "[%%iso8601%%] [%%priority%%] [%%thread%%] %%message%%";
const std::string target_file::DEFAULT_FORMATTER_DEBUGINFO = "[%%iso8601%%] [%%priority%%] [%%thread%%] [%%file%%/%%line%% %%function%%] %%message%%";

target_file::target_file(const std::string a_filename, prio_t a_threshold, std::string a_format)
: target_base(a_threshold, a_format)
{
	set_enable_color(false);
	m_logfile.open(a_filename.c_str(), std::ios::app | std::ios::ate);
	if (!m_logfile.is_open()) {
		std::runtime_error e("ss::log::target_file: Unable to open log file.");
		throw(e);
	}
}

target_file::~target_file()
{
	m_logfile.close();
}

void target_file::post_logtext(std::string& a_formatted_message)
{
	m_logfile << a_formatted_message << std::endl;
}

// log context

ctx& ctx::get()
{
	static ctx shared_instance;
	return shared_instance;
}

void ctx::add_target(std::shared_ptr<target_base> a_target, const std::string& a_name)
{
	std::lock_guard<std::mutex> l_guard(m_system_mutex);
	m_targets.insert(std::make_pair(a_name, a_target));
}

void ctx::remove_target(const std::string& a_name)
{
	std::lock_guard<std::mutex> l_guard(m_system_mutex);
	m_targets.erase(m_targets.find(a_name));
}

void ctx::register_thread(const std::string& a_thread_name)
{
	std::lock_guard<std::mutex> l_guard(m_system_mutex);
	m_threads.insert(std::pair<std::thread::id, std::string>(std::this_thread::get_id(), a_thread_name));
//	std::cout << "m_threads has " << m_threads.size() << " members now." << std::endl;
}

void ctx::unregister_thread()
{
	std::lock_guard<std::mutex> l_guard(m_system_mutex);
	m_threads.erase(m_threads.find(std::this_thread::get_id()));
//	std::cout << "m_threads has " << m_threads.size() << " members now." << std::endl;
}

void ctx::log(std::string a_message, const std::source_location loc)
{
	std::lock_guard<std::mutex> l_guard(m_system_mutex);
	std::string l_tn;
	std::unordered_map<std::thread::id, std::string>::iterator l_it = m_threads.find(std::this_thread::get_id());
	if (l_it == m_threads.end()) {
		l_tn = "none";
	} else {
		l_tn = l_it->second;
	}
	for (const auto& [key, value] : m_targets) {
		value->accept_logtext(a_message, l_tn, loc);
	}
}

void ctx::log_p(prio_t a_priority, std::string a_message, const std::source_location loc)
{
	std::lock_guard<std::mutex> l_guard(m_system_mutex);
	std::string l_tn;
	std::unordered_map<std::thread::id, std::string>::iterator l_it = m_threads.find(std::this_thread::get_id());
	if (l_it == m_threads.end()) {
		l_tn = "none";
	} else {
		l_tn = l_it->second;
	}
	for (const auto& [key, value] : m_targets) {
		value->accept_logtext_p(a_priority, a_message, l_tn, loc);
	}
}

void ctx::set_p(prio_t a_priority)
{
	std::lock_guard<std::mutex> l_guard(m_system_mutex);
	for (const auto& [key, value] : m_targets) {
		value->set_p(a_priority);
	}
}

} // namespace log
} // namespace ss

