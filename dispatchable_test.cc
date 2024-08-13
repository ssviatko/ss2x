#include <iostream>
#include <chrono>

#include "dispatchable.h"

class manager : public ss::ccl::dispatchable {
public:
	manager();
	virtual ~manager();
	virtual bool dispatch();
	virtual void starting();
	virtual void started();
	virtual void halting();
	virtual void halted();
};

manager::manager()
: dispatchable()
{
}

manager::~manager()
{
}

void manager::starting()
{
	std::cout << "Manager start request received..." << std::endl;
}

void manager::started()
{
	std::cout << "Manager started." << std::endl;
}

void manager::halting()
{
	std::cout << "Manager halt request received...." << std::endl;
}

void manager::halted()
{
	std::cout << "Manager halted." << std::endl;
}

bool manager::dispatch()
{
	snooze();
	std::cout << "manager::dispatch: doing nothing and loving it" << std::endl;
	return true;
}

int main(int argc, char **argv)
{
	std::cout << "creating manager instance..." << std::endl;
	manager m;
	std::cout << "starting manager..." << std::endl;
	m.start();
	std::cout << "sleeping..." << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "stopping manager..." << std::endl;
	m.halt();
	return 0;
}

