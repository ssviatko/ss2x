#include <iostream>
#include <chrono>

#include "dispatchable.h"

class manager : public ss::ccl::dispatchable {
public:
	manager();
	virtual ~manager();
	virtual bool dispatch();
};

manager::manager()
: dispatchable()
{
}

manager::~manager()
{
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

