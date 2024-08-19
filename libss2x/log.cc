#include "log.h"

namespace ss {

namespace log {

// target_base

target_base::target_base(prio_t a_threshold, std::string a_format)
: m_priority(DEBUG)
, m_threshold(a_threshold)
, m_format(a_format)
, m_enable_color(true)
{
	
}

target_base::~target_base()
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

// syslog

const std::string target_syslog::DEFAULT_FORMATTER = "[%%thread%%] %%message%%";
const std::string target_syslog::DEFAULT_FORMATTER_DEBUGINFO = "[%%thread%%] [%%file%%/%%line%% %%function%%] %%message%%";

target_syslog::target_syslog(prio_t a_threshold, std::string a_format, const char *a_ident)
: target_base(a_threshold, a_format)
{
	set_enable_color(false);
	openlog(a_ident, LOG_PID | LOG_CONS | LOG_NDELAY, LOG_USER);
}

target_syslog::~target_syslog()
{
	closelog();
}

void target_syslog::post_logtext(std::string& a_formatted_message)
{
	syslog(m_priority, "%s", a_formatted_message.c_str());
}

// file

const std::string target_file::DEFAULT_FORMATTER = "[%%iso8601%%] [%%priority%%] [%%thread%%] %%message%%";
const std::string target_file::DEFAULT_FORMATTER_DEBUGINFO = "[%%iso8601%%] [%%priority%%] [%%thread%%] [%%file%%/%%line%% %%function%%] %%message%%";

target_file::target_file(const std::string a_filename, prio_t a_threshold, std::string a_format)
: target_base(a_threshold, a_format)
, m_logfile_name(a_filename)
, m_rotator_enabled(false)
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

void target_file::set_enable_rotator(const std::uint64_t a_max_size)
{
	m_rotator_max_size = a_max_size;
	m_rotator_enabled = true;
}

void target_file::post_logtext(std::string& a_formatted_message)
{
	m_logfile << a_formatted_message << std::endl;
	m_logfile.flush();
	if (m_rotator_enabled) {
		std::filesystem::path l_file(m_logfile_name);
		if (std::filesystem::file_size(l_file) > m_rotator_max_size) {
			ss::failure_services& l_fs = ss::failure_services::get();
			l_fs.temporarily_ignore_signals();
			m_logfile.close();
			std::stringstream l_arc;
			l_arc << "tar -czf " << m_logfile_name << "-" << doubletime::now_as_file_stamp() << ".tar.gz " << m_logfile_name;
			int l_tar = std::system(l_arc.str().c_str());
			if (l_tar != 0) {
				std::stringstream l_error;
				l_error << "ss::log::target_file: unable to archive log file, tar returned " << l_tar;
				throw std::runtime_error(l_error.str());
			}
			if (!(std::filesystem::remove(m_logfile_name))) {
				throw std::runtime_error("ss::log::target_file: can't delete log file after archiving");
			}
			m_logfile.open(m_logfile_name.c_str(), std::ios::app | std::ios::ate);
			if (!m_logfile.is_open()) {
				throw std::runtime_error("ss::log::target_file: Unable to reopen log file.");
			}
			l_fs.unignore_signals();
		}
	}
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

