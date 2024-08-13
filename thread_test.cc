#include <iostream>
#include <string>
#include <chrono>

#include "ccl.h"
#include "log.h"

class worker : public ss::ccl::thread {
public:
	worker(const std::string& a_logname) : ss::ccl::thread(a_logname) { }
	virtual void execute(ss::log::ctx& ctx);
};

void worker::execute(ss::log::ctx& ctx)
{
	while (!is_stop_requested()) {
		ctx.log("worker thread doing nothing");
		snooze();
	}
}

int main(int argc, char **argv)
{
	ss::log::ctx& ctx = ss::log::ctx::get();
	ctx.register_thread("main");
	std::shared_ptr<ss::log::target_stdout> l_stdout =
		std::make_shared<ss::log::target_stdout>(ss::log::DEBUG, ss::log::target_stdout::DEFAULT_FORMATTER);
	ctx.add_target(l_stdout, "default");
	worker w1("worker1");
	worker w2("worker2");
	ctx.log("starting worker threads..");
	w1.start();
	w2.start();
	ctx.log("main thread sleeping...");
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
	w1.request_stop();
	w2.request_stop();
	ctx.log("joining worker threads...");
	w1.join();
	w2.join();
	return 0;
}
