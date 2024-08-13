#ifndef CCL_H
#define CCL_H

#include <string>
#include <thread>
#include <atomic>
#include <chrono>

#include "log.h"

namespace ss {
namespace ccl {

class thread {
	void execute_core();
	
public:
	thread(const std::string& a_logname);
	void start();
	virtual void execute(ss::log::ctx& ctx) = 0;
	void request_stop();
	bool is_stop_requested();
	void join();

protected:
	std::thread m_thread;
	std::string m_thread_name;
	std::atomic<bool> m_stop_requested;
	void snooze();
};

} // namespace ccl
} // namespace ss

#endif //CCL_H

