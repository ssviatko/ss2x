#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <thread>
#include <format>
#include <source_location>
#include <fstream>

namespace ss {

const std::string COLOR_BLACK = "\033[30m";
const std::string COLOR_RED = "\033[31m";
const std::string COLOR_GREEN = "\033[32m";
const std::string COLOR_YELLOW = "\033[33m";
const std::string COLOR_BLUE = "\033[34m";
const std::string COLOR_MAGENTA = "\033[35m";
const std::string COLOR_CYAN = "\033[36m";
const std::string COLOR_LIGHTGRAY = "\033[37m";
const std::string COLOR_DARKGRAY = "\033[90m";
const std::string COLOR_LIGHTRED = "\033[91m";
const std::string COLOR_LIGHTGREEN = "\033[92m";
const std::string COLOR_LIGHTYELLOW = "\033[93m";
const std::string COLOR_LIGHTBLUE = "\033[94m";
const std::string COLOR_LIGHTMAGENTA = "\033[95m";
const std::string COLOR_LIGHTCYAN = "\033[96m";
const std::string COLOR_WHITE = "\033[97m";
const std::string COLOR_DEFAULT = "\033[39m";

namespace log {

enum prio_t {
	EMERG = 0, // Emergency
	ALERT, // Alert
	CRIT, // Critical
	ERR, // Error
	WARNING, // Warning
	NOTICE, // Notice
	INFO, // Info
	DEBUG // Debug
};

const std::unordered_map<std::string, std::string> color_tokens = {
	{ "%%COLORBLACK%%", COLOR_BLACK },
	{ "%%COLORRED%%", COLOR_RED },
	{ "%%COLORGREEN%%", COLOR_GREEN },
	{ "%%COLORYELLOW%%", COLOR_YELLOW },
	{ "%%COLORBLUE%%", COLOR_BLUE },
	{ "%%COLORMAGENTA%%", COLOR_MAGENTA },
	{ "%%COLORCYAN%%", COLOR_CYAN },
	{ "%%COLORLIGHTGRAY%%", COLOR_LIGHTGRAY },
	{ "%%COLORDARKGRAY%%", COLOR_DARKGRAY },
	{ "%%COLORLIGHTRED%%", COLOR_LIGHTRED },
	{ "%%COLORLIGHTGREEN%%", COLOR_LIGHTGREEN },
	{ "%%COLORLIGHTYELLOW%%", COLOR_LIGHTYELLOW },
	{ "%%COLORLIGHTBLUE%%", COLOR_LIGHTBLUE },
	{ "%%COLORLIGHTMAGENTA%%", COLOR_LIGHTMAGENTA },
	{ "%%COLORLIGHTCYAN%%", COLOR_LIGHTCYAN },
	{ "%%COLORWHITE%%", COLOR_WHITE },
	{ "%%COLORDEFAULT%%", COLOR_DEFAULT }
};

std::string iso8601();

class target_base {
public:
	target_base(prio_t a_threshold, std::string a_format);
	void accept_logtext(std::string a_line, std::string a_thread_name, const std::source_location& a_location);
	void accept_logtext_p(prio_t a_priority, std::string a_line, std::string a_thread_name, const std::source_location& a_location);
	virtual void post_logtext(std::string& a_formatted_message) = 0;
	void set_p(prio_t a_priority);
	void set_enable_color(bool a_enable);
	
protected:
	prio_t m_priority;
	prio_t m_threshold;
	std::string m_format;
	bool m_enable_color;
};

class target_stdout : public target_base {
public:
	target_stdout(prio_t a_threshold, std::string a_format);
	virtual ~target_stdout();
	virtual void post_logtext(std::string& a_formatted_message);
	const static std::string DEFAULT_FORMATTER;
	const static std::string DEFAULT_FORMATTER_DEBUGINFO;
};

class target_file : public target_base {
public:
	target_file(const std::string a_filename, prio_t a_threshold, std::string a_format);
	virtual ~target_file();
	virtual void post_logtext(std::string& a_formatted_message);
	const static std::string DEFAULT_FORMATTER;
	const static std::string DEFAULT_FORMATTER_DEBUGINFO;

protected:
	std::ofstream m_logfile;
};

class ctx {
public:
	static ctx& get();
	void register_thread(const std::string& a_thread_name);
	void unregister_thread();
	void add_target(std::shared_ptr<target_base> a_target, const std::string& a_name);
	void remove_target(const std::string& a_name);
	void log(std::string a_message, const std::source_location loc = std::source_location::current());
	void log_p(prio_t a_priority, std::string a_message, const std::source_location loc = std::source_location::current());
	void set_p(prio_t a_priority);
	
protected:
	std::unordered_map<std::string, std::shared_ptr<target_base> > m_targets;
	std::unordered_map<std::thread::id, std::string> m_threads;
	std::mutex m_system_mutex;
};

} // namespace log
} // namespace ss

#endif // LOG_H

