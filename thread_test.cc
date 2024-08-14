#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <format>

#include "ccl.h"
#include "log.h"
#include "fs.h"

class worker : public ss::ccl::thread {
public:
	worker(const std::string& a_logname) : ss::ccl::thread(a_logname) { }
	virtual void execute();
};

void worker::execute()
{
	while (!is_stop_requested()) {
		ctx.log("worker thread doing nothing");
		snooze();
	}
}

class queue_worker : public ss::ccl::work_queue_thread<std::string> {
public:
	queue_worker(const std::string& a_logname, ss::ccl::work_queue<std::string>& a_queue);
	virtual void dispatch(std::string a_work_item);
};

queue_worker::queue_worker(const std::string& a_logname, ss::ccl::work_queue<std::string>& a_queue)
: ss::ccl::work_queue_thread<std::string>(a_logname, a_queue)
{
}

void queue_worker::dispatch(std::string a_work_item)
{
	ctx.log(std::format("queue_worker: got string {}", a_work_item));
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main(int argc, char **argv)
{
	ss::failure_services& fs = ss::failure_services::get();
	fs.install_signal_handler();
	
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
	
	// test work_queue
	ss::ccl::work_queue<std::string> l_q;
	l_q.add_work_item("Hello");
	l_q.add_work_item("World");
	ctx.log(std::format("work_queue is {} items long.", l_q.queue_size()));
	ctx.log(std::format("wait_for_item: {}", l_q.wait_for_item(500).value_or("none")));
	ctx.log(std::format("wait_for_item: {}", l_q.wait_for_item(500).value_or("none")));
	std::string l_ghost_item = l_q.wait_for_item(500).value_or("bailed out");
	ctx.log(std::format("wait_for_item_timed return {}", l_ghost_item));
	
	// spin up two work_queue_threads and point them to our work queue. Periodically feed them data.
	queue_worker qw1("qw1", l_q);
	queue_worker qw2("qw2", l_q);
	qw1.start();
	qw2.start();
	for (std::size_t i = 0; i < 10; ++i) {
		std::stringstream l_ss;
		l_ss << "somestring" << i;
		ctx.log(std::format("adding work item {} to the queue...", i));
		l_q.add_work_item(l_ss.str());
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	while (!l_q.wait_for_empty(500))
		ctx.log("waiting for queue to empty...");
	ctx.log("shutting down work queue");
	l_q.shut_down();
	qw1.join();
	qw2.join();
	
	return 0;
}
