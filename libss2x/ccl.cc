#include "ccl.h"

namespace ss {
namespace ccl {

thread::thread(const std::string& a_logname)
: m_thread_name(a_logname)
, m_stop_requested(false)
{
}

void thread::start()
{
	m_thread = std::thread(&thread::execute_core, this);
}

void thread::join()
{
	m_thread.join();
}

void thread::request_stop()
{
	m_stop_requested = true;
}

bool thread::is_stop_requested()
{
	return m_stop_requested;
}

void thread::snooze()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

void thread::execute_core()
{
	ctx.register_thread(m_thread_name);
	execute();
	ctx.unregister_thread();
}

} // namespace ccl
} // namespace ss

