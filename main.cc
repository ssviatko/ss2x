#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <format>
#include <thread>

#include "log.h"
#include "icr.h"
#include "data.h"

void threadfunc(std::string myname)
{
	ss::log::ctx& ctx = ss::log::ctx::get();
	ctx.register_thread(myname);
	for (int i = 0; i < 4; ++i) {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		ctx.log(std::format("hello from inside thread {}", i));
	}
	ctx.log_p(ss::log::WARNING, "thread is exiting");
	ctx.unregister_thread();
}

int main(int argc, char **argv)
{
	std::cout << "ss2x framework test" << std::endl;
	std::cout << "build no: " << BUILD_NUMBER << " release: " << RELEASE_NUMBER << " built on: " << BUILD_DATE << std::endl;

	std::cout << "ss2xlog logging framework test" << std::endl;

	ss::log::ctx& ctx = ss::log::ctx::get();
	
	// register main thread
	ctx.register_thread("main");
	
	// configure our target(s)
	std::shared_ptr<ss::log::target_stdout> l_stdout =
		std::make_shared<ss::log::target_stdout>(ss::log::DEBUG, ss::log::target_stdout::DEFAULT_FORMATTER_DEBUGINFO);
	ctx.add_target(l_stdout, "default");
	
	// set systemwide default log priority
	ctx.set_p(ss::log::DEBUG);
	
	ctx.log("Hello there");
	ctx.log("another message");
	for (int i = 4; i <= 7; ++i)
		ctx.log(std::format("Counting loop: variable is {} now.", i));
	ctx.log_p(ss::log::NOTICE, "temporarily printing a notice");
	ctx.log("and back to normal");
	ctx.set_p(ss::log::INFO);
	ctx.log("let's print a couple of infos");
	ctx.log("that should do it");
	ctx.set_p(ss::log::DEBUG);
	ctx.log("and now back to debug as normal");
	ctx.log("kill colors");
	l_stdout->set_enable_color(false);
	ctx.log("this should be in monochrome");
	l_stdout->set_enable_color(true);
	ctx.log("spin up a file logger to ss2x.log");
	std::shared_ptr<ss::log::target_file> l_file = std::make_shared<ss::log::target_file>("ss2x.log", ss::log::DEBUG, ss::log::target_file::DEFAULT_FORMATTER_DEBUGINFO);
	ctx.add_target(l_file, "logfile");
	std::thread t1(&threadfunc, "t1");
	std::thread t2(&threadfunc, "t2");
	t1.join();
	t2.join();

// don't do this... the log message will be imbued with color even if you are printing to a target with color turned off (like the syslog, or a file)
//	ctx.log_p(ss::log::NOTICE, std::format("Log messages with {}C {}O {}L {}O {}R {}S {} are neat!", ss::COLOR_LIGHTRED, ss::COLOR_LIGHTYELLOW, ss::COLOR_LIGHTGREEN, ss::COLOR_GREEN, ss::COLOR_LIGHTBLUE, ss::COLOR_LIGHTMAGENTA, ss::COLOR_DEFAULT));

// do this instead
	ctx.log_p(ss::log::NOTICE, "Log messages with %%COLORLIGHTRED%%C %%COLORLIGHTYELLOW%%O %%COLORLIGHTGREEN%%L %%COLORGREEN%%O %%COLORLIGHTBLUE%%R %%COLORLIGHTMAGENTA%%S %%COLORDEFAULT%% are neat!");

	ctx.log("discontinuing logfile...");
	ctx.remove_target("logfile");
	l_file.reset(); // delete and invalidate shared pointer
	
	// test ICR
	ctx.log_p(ss::log::INFO, "Testing ICR... loading ss2x.ini file");
	ss::icr& icr = ss::icr::get();
	icr.read_file("ss2x.ini", true);
	icr.read_arguments(argc, argv);
	ctx.log_p(ss::log::INFO, std::format("We are running as {}", icr.exename()));
	
	std::vector<std::string> l_cats = icr.categories();
	ctx.log(std::format("There are {} categories.", l_cats.size()));
	for (auto& i : l_cats) {
		ctx.log(std::format("found category: {}", i));
		std::vector<std::string> l_keyvalues = icr.keys_for_category(i);
		for (auto& j : l_keyvalues) {
			ctx.log(std::format("key {} is {}", j, icr.keyvalue(i, j)));
		}
	}
	
	// test ss::data
	ss::data a;
	a.set_network_byte_order(true);
	a.write_uint8(0xff);
	a.write_uint8(0xaa);
	a.write_uint8(0xdd);
	
	for (int i = 0; i < 3; ++i) {
		ctx.log(std::format("data: a read {:x}", a.read_uint8()));
	}
	
	a.write_uint32(0xdeadbeef);
	a.write_uint32(0xc0edbabe);
	a.write_uint32(0x12345678);
	
	for (int i = 0; i < 3; ++i) {
		ctx.log(std::format("data: a read {:x}", a.read_uint32()));
	}
	ctx.log(a.as_hex_str());
	a.set_write_cursor(4);
	a.write_uint32(0x0);
	ctx.log(a.as_hex_str());
	a.set_write_cursor(13);
	a.write_uint32(0x99999999);
	ctx.log(a.as_hex_str());
	ctx.log(std::format("a has a crc32 of {:x}", a.crc32(0)));
	a.save_file("a.hex");
	ctx.log("saving to file a.hex");
	ss::data c;
	c.load_file("main.cc");
	ctx.log(std::format("loaded main.cc, file size is {}, crc32 is {:x}.", c.size(), c.crc32(0)));
	ctx.log(std::format("md5: {}", c.md5().as_hex_str_nospace()));
	ctx.log(std::format("sha1: {}", c.sha1().as_hex_str_nospace()));
	ctx.log(std::format("sha2_224: {}", c.sha2_224().as_hex_str_nospace()));
	ctx.log(std::format("sha2_256: {}", c.sha2_256().as_hex_str_nospace()));
	ctx.log(std::format("sha2_384: {}", c.sha2_384().as_hex_str_nospace()));
	ctx.log(std::format("sha2_512: {}", c.sha2_512().as_hex_str_nospace()));
	
	ss::data b;
	b.write_int8(-3);
	ctx.log(std::format("write -3, read as uint8: {}", b.read_uint8()));

	ss::data d;
	d.write_int32(-10);
	std::int32_t dd = d.read_int32();
	ctx.log(std::format("wrote -10 as int32, read {} (hex is {:x}", dd, static_cast<std::uint32_t>(dd)));
	
	ctx.unregister_thread();
		ctx.log("thread should be missing");
	
	return 0;
}

