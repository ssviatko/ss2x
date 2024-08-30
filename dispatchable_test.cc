#include <iostream>
#include <chrono>
#include <thread>

#include "dispatchable.h"
#include "log.h"
#include "fs.h"

class manager : public ss::ccl::dispatchable {
public:
	manager(const std::string& a_logname);
	virtual ~manager();
	virtual bool dispatch();
	virtual void starting();
	virtual void started();
	virtual void halting();
	virtual void halted();
};

manager::manager(const std::string& a_logname)
: dispatchable(a_logname)
{
}

manager::~manager()
{
}

void manager::starting()
{
	ctx.log("Manager start request received...");
}

void manager::started()
{
	ctx.log("Manager started.");
}

void manager::halting()
{
	ctx.log("Manager halt request received....");
}

void manager::halted()
{
	ctx.log("Manager halted.");
}

bool manager::dispatch()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ctx.log("manager::dispatch: doing nothing and loving it");
	return true;
}

int main(int argc, char **argv)
{
	ss::failure_services& l_fs = ss::failure_services::get();
	l_fs.install_signal_handler();
	ss::log::ctx& ctx = ss::log::ctx::get();
	ctx.register_thread("main");
	std::shared_ptr<ss::log::target_stdout> l_stdout =
		std::make_shared<ss::log::target_stdout>(ss::log::DEBUG, ss::log::target_stdout::DEFAULT_FORMATTER);
	ctx.add_target(l_stdout, "default");
	ctx.log("creating manager instance...");
	manager m("manager");
	
	auto ctrlc = [&]() {
		ctx.log("ctrl-c pressed...");
		ctx.log("stopping manager...");
		m.halt();
		exit(EXIT_SUCCESS);
	};
	
	
	auto hup = [&]() {
		ctx.log("caught SIGHUP...");
	};
	
	l_fs.install_sigint_handler(ctrlc);
	l_fs.install_sighup_handler(hup);
	
	ctx.log("starting manager...");
	m.start();
	ctx.log("sleeping...");
	while (1)
		std::this_thread::sleep_for(std::chrono::seconds(1));
	return 0;
}

