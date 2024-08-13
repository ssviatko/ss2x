#ifndef DISPATCHABLE_H
#define DISPATCHABLE_H

#include <thread>
#include <semaphore>
#include <mutex>
#include <chrono>

namespace ss {
namespace ccl {

class dispatchable {
public:
	dispatchable();
	virtual ~dispatchable();
	void start();
	void halt();
	void snooze();
	virtual bool dispatch() = 0;
	virtual void starting();
	virtual void started();
	virtual void halting();
	virtual void halted();
	
protected:
	void dispatch_core();
	bool m_dispatch_running;
	std::binary_semaphore m_dispatchthr_started { 0 };
	std::binary_semaphore m_dispatchthr_stopped { 0 };
};

} // namespace ccl
} // namespace ss

#endif // DISPATCHABLE_H
