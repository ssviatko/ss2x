#include <iostream>
#include <string>
#include <format>
#include <filesystem>

#include "data.h"
#include "log.h"
#include "fs.h"

int main(int argc, char **argv)
{
	std::cout << "blowfish7 framework test" << std::endl;
	std::cout << "build no: " << BUILD_NUMBER << " release: " << RELEASE_NUMBER << " built on: " << BUILD_DATE << std::endl;

	ss::failure_services& l_fs = ss::failure_services::get();
	l_fs.install_signal_handler();
	
	ss::log::ctx& ctx = ss::log::ctx::get();
	
	// register main thread
	ctx.register_thread("main");
	
	// configure our target(s)
	std::shared_ptr<ss::log::target_stdout> l_stdout =
		std::make_shared<ss::log::target_stdout>(ss::log::DEBUG, ss::log::target_stdout::DEFAULT_FORMATTER_DEBUGINFO);
	ctx.add_target(l_stdout, "default");
	
	// set systemwide default log priority
	ctx.set_p(ss::log::DEBUG);
	
	
	// test blowfish7
	
	ss::data bf7_key1 = ss::data::bf7_key_random();
	ctx.log(std::format("random bf7 key: {}", bf7_key1.as_hex_str_nospace()));
	ss::data bf7_key2 = ss::data::bf7_key_schedule("Stephen Sviatko");
	ctx.log(std::format("scheduled bf7 key: {}", bf7_key2.as_hex_str_nospace()));
	ss::data bf7_key3 = ss::data::bf7_key_schedule("Stephen Sviatko");
	ctx.log(std::format("scheduled bf7 key: {}", bf7_key3.as_hex_str_nospace()));

	ss::data bf7_iv1 = ss::data::bf7_iv_random();
	ctx.log(std::format("random bf7 iv: {}", bf7_iv1.as_hex_str_nospace()));
	ss::data bf7_iv2 = ss::data::bf7_iv_schedule("Stephen Sviatko");
	ctx.log(std::format("scheduled bf7 iv: {}", bf7_iv2.as_hex_str_nospace()));
	ss::data bf7_iv3 = ss::data::bf7_iv_schedule("Stephen Sviatko");
	ctx.log(std::format("scheduled bf7 iv: {}", bf7_iv3.as_hex_str_nospace()));
	
	ss::data bf7_block;
	bf7_block.fill(16, 0);
	ss::data bf7_block_enc = ss::data::bf7_block_encrypt(bf7_block, bf7_key1);
	ctx.log(std::format("block {} enc: {}", bf7_block.as_hex_str_nospace(), bf7_block_enc.as_hex_str_nospace()));
	ss::data bf7_block_dec = ss::data::bf7_block_decrypt(bf7_block_enc, bf7_key1);
	ctx.log(std::format(" dec: {}", bf7_block_dec.as_hex_str_nospace()));
	
	return 0;
}