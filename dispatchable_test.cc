#include <iostream>
#include <chrono>

#include "dispatchable.h"
#include "log.h"

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
	snooze();
	ctx.log("manager::dispatch: doing nothing and loving it");
	return true;
}

int main(int argc, char **argv)
{
	ss::log::ctx& ctx = ss::log::ctx::get();
	ctx.register_thread("main");
	std::shared_ptr<ss::log::target_stdout> l_stdout =
		std::make_shared<ss::log::target_stdout>(ss::log::DEBUG, ss::log::target_stdout::DEFAULT_FORMATTER);
	ctx.add_target(l_stdout, "default");
	ctx.log("creating manager instance...");
	manager m("manager");
	ctx.log("starting manager...");
	m.start();
	ctx.log("sleeping...");
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ctx.log("stopping manager...");
	m.halt();
	return 0;
}

