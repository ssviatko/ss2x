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
	
	auto my_cb1 = [&ctx](std::shared_ptr<ss::ccl::note> a_note) {
		ctx.log(std::format("listener1: got note {} guid {}", a_note->name(), a_note->guid()));
	};
	auto my_cb2 = [&ctx](std::shared_ptr<ss::ccl::note> a_note) {
		ctx.log(std::format("listener2: got note {} guid {}", a_note->name(), a_note->guid()));
	};
	
	// test note dispatcher
	ss::ccl::nd& nd = ss::ccl::nd::get();
	nd.add_listener(ss::ccl::note::SYS_DEFAULT, my_cb1);
	nd.add_listener(ss::ccl::note::SYS_SPECIAL, my_cb1);
	nd.add_listener(ss::ccl::note::SYS_DEFAULT, my_cb2);
	nd.add_listener(ss::ccl::note::SYS_ALTERNATE, my_cb2);
	std::shared_ptr<ss::ccl::note> l_n2 = nd.post(ss::ccl::note::SYS_DEFAULT, false);
	ctx.log(std::format("posted note {}", l_n2->guid()));
	ss::ccl::note_attributes nta;
	nta.set_keyvalue("apple", "red");
	nta.set_keyvalue("banana", "blue");
	std::shared_ptr<ss::ccl::note> l_n3 = nd.post(ss::ccl::note::SYS_DEFAULT, true, nta);
	ctx.log(std::format("posted note {}", l_n3->guid()));
	std::shared_ptr<ss::ccl::note> l_n4 = nd.post(ss::ccl::note::SYS_SPECIAL, false);
	ctx.log(std::format("posted note {}", l_n2->guid()));
	std::shared_ptr<ss::ccl::note> l_n5 = nd.post(ss::ccl::note::SYS_ALTERNATE, false);
	ctx.log(std::format("posted note {}", l_n2->guid()));

	auto my_cb_replier = [&ctx](std::shared_ptr<ss::ccl::note> a_note) {
		ctx.log(std::format("listener replier: got note {} guid {}", a_note->name(), a_note->guid()));
		if (a_note->reply_requested()) {
			a_note->set_reply(ss::ccl::note::REPLY_OK);
		}
	};
	
	nd.add_listener(ss::ccl::note::SYS_REQUEST, my_cb_replier);
	std::shared_ptr<ss::ccl::note> l_nrep = nd.post(ss::ccl::note::SYS_REQUEST, true);
	ctx.log(std::format("posted note {} expecting reply", l_nrep->guid()));
	while (!l_nrep->wait_for_replied(20)) {
		ctx.log("waiting for reply...");
	}
	ctx.log(std::format("listener for note {} posted reply {}", l_nrep->guid(), l_nrep->reply()));

	nd.shutdown();
	
	return 0;
}