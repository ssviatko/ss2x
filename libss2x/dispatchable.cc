#include "dispatchable.h"

namespace ss {
namespace ccl {

dispatchable::dispatchable()
: m_dispatch_running(false)
{
	
}

dispatchable::~dispatchable()
{
	
}

void dispatchable::start()
{
	// start dispatch thread
	m_dispatch_running = true;
	starting();
	std::thread l_dispatch_thr(&dispatchable::dispatch_core, this);
	m_dispatchthr_started.acquire();
	l_dispatch_thr.detach();
	started();
}

void dispatchable::halt()
{
	if (m_dispatch_running) {
		halting();
		m_dispatch_running = false;
		m_dispatchthr_stopped.acquire();
		halted();
	}
}

void dispatchable::starting()
{
}

void dispatchable::started()
{
}

void dispatchable::halting()
{
}

void dispatchable::halted()
{
}

void dispatchable::dispatch_core()
{
	m_dispatchthr_started.release();
	
	while (m_dispatch_running) {
		// if our dispatch routine returns false, break out of here
		if (!dispatch())
			break;
	}
	m_dispatchthr_stopped.release();
}

void dispatchable::snooze()
{
	// snooze for 50 milliseconds if dispatch did nothing
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

} // namespace ccl
} // namespace ss

