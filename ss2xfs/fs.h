#ifndef FS_H
#define FS_H

#include <signal.h>

namespace ss {

class failure_services {
	failure_services();
	~failure_services();
public:
	static failure_services& get();
};

} // namespace ss

#endif // FS_H
