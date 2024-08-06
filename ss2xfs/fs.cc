#include "fs.h"

namespace ss {

failure_services& failure_services::get()
{
	static failure_services shared_instance;
	return shared_instance;
}

}