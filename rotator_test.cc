#include <iostream>
#include <format>

#include <cstdint>
#include <cstdlib>

#include "log.h"
#include "fs.h"

int main(int argc, char **argv)
{
	ss::log::ctx& ctx = ss::log::ctx::get();
	
	// register main thread
	ctx.register_thread("main");
	
	// set systemwide default log priority
	ctx.set_p(ss::log::DEBUG);
	
	ctx.log("spin up a file logger to rotator.log");
	std::shared_ptr<ss::log::target_file> l_file = std::make_shared<ss::log::target_file>("rotator.log", ss::log::DEBUG, ss::log::target_file::DEFAULT_FORMATTER_DEBUGINFO);
	l_file->set_enable_rotator(100000000); // 100MB
	ctx.add_target(l_file, "rotator");

	auto ctrlc = [&]() {
		std::cerr << "Stopping log file generator..." << std::endl;
		std::cerr.flush();
		std::exit(EXIT_SUCCESS);
	};
	
	ss::failure_services& fs = ss::failure_services::get();
	fs.install_signal_handler();
	fs.install_sigint_handler(ctrlc);
	
	std::uint64_t counter = 0;
	do {
		// repetitively shit out log messages to the file just to fill it up
		ctx.log(std::format("This is a verbose log message designed to fill up the log file with crap, it's ID# is {}.", counter++));
		if ((counter % 100000) == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		std::this_thread::yield();
	} while (1);
	
	return 0;
}