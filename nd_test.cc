#include <iostream>
#include <string>
#include <format>

#include "nd.h"
#include "log.h"
#include "fs.h"

class note_thread : public ss::ccl::thread {
public:
	note_thread();
	~note_thread() { }
	virtual void execute();
	ss::ccl::note note_latch;
};

note_thread::note_thread()
: ss::ccl::thread("note_thread")
{
	
}

void note_thread::execute()
{
	while (!is_stop_requested()) {
		if (!wait_for_trigger())
			continue; // stop requested while waiting for trigger
		ctx.log(std::format("received note n1 name = {}, guid = {}", note_latch.name(), note_latch.guid()));
		// work on it for a while
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		if (note_latch.reply_requested()) {
			note_latch.set_reply(ss::ccl::note::REPLY_OK);
		}
	}
	ctx.log("note thread quitting...");
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
	
	note_thread nthr;
	nthr.start();
	
	// send 5 notes
	for (std::size_t i = 0; i < 5; ++i) {
		ss::ccl::note n1(ss::ccl::note::SYS_DEFAULT);
		n1.set_reply_requested();
		nthr.note_latch = n1;
		nthr.trigger();
		while (!nthr.note_latch.wait_for_replied(50)) {
			ctx.log("waiting for reply...");
		}
		ctx.log(std::format("note name = {}, reply = {}", nthr.note_latch.name(), nthr.note_latch.reply()));
	}
	
	nthr.request_stop();
	nthr.join();
	
	return 0;
}