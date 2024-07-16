#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <format>
#include <thread>
#include <array>
#include <filesystem>

#include "log.h"
#include "icr.h"
#include "data.h"

typedef struct {
	std::uint64_t key;
	std::uint64_t clear;
	std::uint64_t enc;
} bf_block_test_vector;

std::array<bf_block_test_vector, 15> bf_vec = {{
	{ 0x0000000000000000ULL, 0x0000000000000000ULL, 0x4EF997456198DD78ULL },
	{ 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0x51866FD5B85ECB8AULL },
	{ 0x3000000000000000ULL, 0x1000000000000001ULL, 0x7D856F9A613063F2ULL },
	{ 0x1111111111111111ULL, 0x1111111111111111ULL, 0x2466DD878B963C9DULL },
	{ 0x0123456789ABCDEFULL, 0x1111111111111111ULL, 0x61F9C3802281B096ULL },
	{ 0x3000000000000000ULL, 0x1000000000000001ULL, 0x7D856F9A613063F2ULL },
	{ 0x1111111111111111ULL, 0x1111111111111111ULL, 0x2466DD878B963C9DULL },
	{ 0x0123456789ABCDEFULL, 0x1111111111111111ULL, 0x61F9C3802281B096ULL },
	{ 0x1111111111111111ULL, 0x0123456789ABCDEFULL, 0x7D0CC630AFDA1EC7ULL },
	{ 0x0000000000000000ULL, 0x0000000000000000ULL, 0x4EF997456198DD78ULL },
	{ 0xFEDCBA9876543210ULL, 0x0123456789ABCDEFULL, 0x0ACEAB0FC6A0A28DULL },
	{ 0x7CA110454A1A6E57ULL, 0x01A1D6D039776742ULL, 0x59C68245EB05282BULL },
	{ 0x0131D9619DC1376EULL, 0x5CD54CA83DEF57DAULL, 0xB1B8CC0B250F09A0ULL },
	{ 0x07A1133E4A0B2686ULL, 0x0248D43806F67172ULL, 0x1730E5778BEA1DA4ULL },
	{ 0x3849674C2602319EULL, 0x51454B582DDF440AULL, 0xA25E7856CF2651EBULL }
}};

/*
more of them if you want
04B915BA43FEB5B6        42FD443059577FA2        353882B109CE8F1A
0113B970FD34F2CE        059B5E0851CF143A        48F4D0884C379918
0170F175468FB5E6        0756D8E0774761D2        432193B78951FC98
43297FAD38E373FE        762514B829BF486A        13F04154D69D1AE5
07A7137045DA2A16        3BDD119049372802        2EEDDA93FFD39C79
04689104C2FD3B2F        26955F6835AF609A        D887E0393C2DA6E3
37D06BB516CB7546        164D5E404F275232        5F99D04F5B163969
1F08260D1AC2465E        6B056E18759F5CCA        4A057A3B24D3977B
584023641ABA6176        004BD6EF09176062        452031C1E4FADA8E
025816164629B007        480D39006EE762F2        7555AE39F59B87BD
49793EBC79B3258F        437540C8698F3CFA        53C55F9CB49FC019
4FB05E1515AB73A7        072D43A077075292        7A8E7BFA937E89A3
49E95D6D4CA229BF        02FE55778117F12A        CF9C5D7A4986ADB5
018310DC409B26D6        1D9D5C5018F728C2        D1ABB290658BC778
1C587F1C13924FEF        305532286D6F295A        55CB3774D13EF201
0101010101010101        0123456789ABCDEF        FA34EC4847B268B2
1F1F1F1F0E0E0E0E        0123456789ABCDEF        A790795108EA3CAE
E0FEE0FEF1FEF1FE        0123456789ABCDEF        C39E072D9FAC631D
0000000000000000        FFFFFFFFFFFFFFFF        014933E0CDAFF6E4
FFFFFFFFFFFFFFFF        0000000000000000        F21E9A77B71C49BC
0123456789ABCDEF        0000000000000000        245946885754369A
FEDCBA9876543210        FFFFFFFFFFFFFFFF        6B5C5A9C5D9E0A5A
*/

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
	
	// test 16 bit functions
	ss::data e;
	e.set_network_byte_order(true);
	e.write_uint16(65535);
	e.write_uint16(65534);
	e.write_int16(-3);
	e.write_int16(-4);
	ctx.log(std::format("e contains {}", e.as_hex_str()));
	// ludicrous... std;:format is calling these in reverse order!
	ctx.log(std::format("numbers are {:x} {:x} {:x} {:x} {:x} {:x} {}", e.read_uint16(), e.read_uint8(), e.read_uint8(), e.read_uint16(), e.read_int16(), 0x9dbf, -112));
	ctx.log(std::format("e read cursor: {} write cursor: {}", e.get_read_cursor(), e.get_write_cursor()));
	e.set_read_cursor(0);
	for (int i = 0; i < 4; ++i)
		ctx.log(std::format("sequence {} is {:x}", i, e.read_uint16()));
		
	// test 64 bit functions
	ss::data f;
	f.write_uint64(0xdeadbeefbeaddeef);
	f.write_uint64(23);
	f.write_int64(-2);
	f.write_int64(-1000000);
	ctx.log(std::format("f contains {}", f.as_hex_str()));
	for (int i = 0; i <= 3; ++i)
		ctx.log(std::format("seq {} is {:x}", i, f.read_uint64()));
	
	// test floats, doubles, long doubles
	ss::data g;
	g.write_float(3.1415926535F);
	g.write_double(3.1415927653543212345678);
	g.write_longdouble(3.141592653543212345678L);
	ctx.log(std::format("g contains {}", g.as_hex_str()));
	ctx.log(std::format("read float: {}", g.read_float()));
	ctx.log(std::format("read double: {}", g.read_double()));
	ctx.log(std::format("read long double: {}", g.read_longdouble()));
	g.clear();
	g.set_network_byte_order(true);
	g.write_float(3.1415926535F);
	g.write_double(3.1415927653543212345678);
	g.write_longdouble(3.141592653543212345678L);
	ctx.log(std::format("(network byte order) g contains {}", g.as_hex_str()));
	ctx.log(std::format("read float: {}", g.read_float()));
	ctx.log(std::format("read double: {}", g.read_double()));
	ctx.log(std::format("read long double: {}", g.read_longdouble()));
	
	// test random fill feature
	ss::data h;
	for (int i = 0; i < 8; ++i) {
		h.random(16);
		ctx.log(std::format("random 16 bytes: {}", h.as_hex_str_nospace()));
		h.clear();
	}
	
	// test strings
	ss::data str_test;
	str_test.write_std_str("Now is time for all good men.");
	ctx.log(std::format("write_std_str: {}", str_test.as_hex_str()));
	ss::data str_test_delim;
	str_test_delim.write_std_str_delim("Now is time for all good men.");
	ctx.log(std::format("write_std_str_delim: {}", str_test_delim.as_hex_str()));
	std::string l_read_str = str_test.read_std_str(9);
	ctx.log(std::format("read_std_str(9): {} size={}", l_read_str, l_read_str.size()));
	l_read_str = str_test.read_std_str(10);
	ctx.log(std::format("read_std_str(10): {} size={}", l_read_str, l_read_str.size()));
	str_test_delim.write_std_str_delim("That was the time then,");
	str_test_delim.write_std_str_delim("This is the time now.");
	str_test_delim.write_std_str("And now with no delimiter on the end");
	for (int i = 0; i <= 3; ++i) {
		l_read_str = str_test_delim.read_std_str_delim().value_or("no delimiter found");
		ctx.log(std::format("read_std_str_delim: {} size={}", l_read_str, l_read_str.size()));
	}
	
	// comparison
	ss::data compare1;
	compare1.write_int64(-1024);
	ss::data compare2;
	compare2.write_int64(-1024);
	ss::data compare3;
	compare3.write_int64(-55554);
	ctx.log(std::format("compare1 == compare2 {}", compare1 == compare2));
	ctx.log(std::format("compare2 == compare3 {}", compare2 == compare3));
	ctx.log(std::format("compare1 == compare3 {}", compare1 == compare3));
	ctx.log(std::format("compare1 != compare3 {}", compare1 != compare3));
	
	// test encryption suite
	
	ss::data enc_randkey = ss::data::bf_key_random();
	ctx.log(std::format("BF: random 448 bit key: {}", enc_randkey.as_hex_str_nospace()));
	ss::data enc_schedule = ss::data::bf_key_schedule("Stephen Sviatko");
	ctx.log(std::format("BF: scheduled key1: {}", enc_schedule.as_hex_str_nospace()));
	ss::data enc_schedule2 = ss::data::bf_key_schedule("Stephen Sviatko");
	ctx.log(std::format("BF: scheduled key2: {}", enc_schedule2.as_hex_str_nospace()));
	
	// round trip
	ss::data enc_rt_clear;
	enc_rt_clear.random(8);
	ss::data enc_rt_key = ss::data::bf_key_random();
	ss::data enc_rt_enc;
	enc_rt_enc = ss::data::bf_block_encrypt(enc_rt_clear, enc_rt_key);
	ss::data enc_rt_dec;
	enc_rt_dec = ss::data::bf_block_decrypt(enc_rt_enc, enc_rt_key);
	ctx.log(std::format("BF: round trip key: {} clear: {} enc: {} dec: {} check: {}", enc_rt_key.as_hex_str_nospace(), enc_rt_clear.as_hex_str_nospace(), enc_rt_enc.as_hex_str_nospace(), enc_rt_dec.as_hex_str_nospace(), enc_rt_clear == enc_rt_dec));
	
	// test vectors
	for (auto& i : bf_vec) {
		ss::data l_key;
		l_key.set_network_byte_order(true); // so we don't write the integer in reverse order
		l_key.write_uint64(i.key);
		ss::data l_clear;
		l_clear.set_network_byte_order(true);
		l_clear.write_uint64(i.clear);
		ss::data l_enc = ss::data::bf_block_encrypt(l_clear, l_key);
		l_enc.set_network_byte_order(true); // our returned block will be in sequential (network) byte order.. forcibly reverse it so we can read an integer
		std::uint64_t l_readback = l_enc.read_uint64();
		bool l_check = (l_readback == i.enc);
//		std::cout << "enc " << std::hex << std::setfill('0') << std::setw(16) << l_readback << " check " << bf_vec[i].enc << std::endl;
		ctx.log(std::format("key {} clear {} enc {} check {}", l_key.as_hex_str_nospace(), l_clear.as_hex_str_nospace(), l_enc.as_hex_str_nospace(), l_check));
	}
	
	// textual presentation test
	ss::data tp1;
	tp1.write_hex_str("9dbfdeadbeefc0edbabeffffff000001");
	ctx.log(std::format("tp1 is: {}", tp1.as_hex_str_nospace()));
	tp1.set_read_cursor(2);
	ctx.log(std::format("snippet of tp1 is: {}", tp1.read_hex_str(9)));
	tp1.set_read_cursor(0);
	std::string l_tp1b64 = tp1.read_base64(tp1.size());
	ctx.log(l_tp1b64);
	tp1.set_write_cursor_to_append();
	tp1.write_base64(l_tp1b64);
	tp1.write_base64(l_tp1b64);
	ctx.log(std::format("tp1 x3: {}", tp1.as_hex_str_nospace()));
	std::string l_tp3 = tp1.as_base64();
	ctx.log(l_tp3);
	ss::data tp2;
	tp2.from_base64(l_tp3);
	ctx.log(std::format("tp2 x3: {}", tp2.as_hex_str_nospace()));
	
	// bits etc
	ss::data bit1;
	std::string l_bit1_from = "11100011 10011001 00011101 11010011 11011";
	bit1.from_bits(l_bit1_from);
	std::string l_bit1 = bit1.as_bits();
	ctx.log(std::format("bit1 from is {}", l_bit1_from));
	ctx.log(std::format("bit1 as   is {}", l_bit1));
	std::uint64_t l_bit1_read = bit1.read_bits(37);
	ctx.log(std::format("bit1 read_bits(37) is: {:x}", l_bit1_read));
	ctx.log(std::format("sanity check bit1: {} size={}", bit1.as_hex_str_nospace(), bit1.size()));
	
	// data compression
	
	// test one byte file
	ss::data htzero;
//	htzero.set_huffman_debug(true);
	htzero.write_uint8(40);
	ss::data htzero_comp = htzero.huffman_encode();
	ctx.log(std::format("encoded 1 length file, len={}", htzero_comp.size()));
	ss::data htzero_decomp = htzero_comp.huffman_decode();
	ctx.log(std::format("decoded 1 length file, len={} check {}", htzero_decomp.size(), (htzero_decomp == htzero)));

	// test repeated data edge case (single token)
	ss::data htrep;
//	htrep.set_huffman_debug(true);
	htrep.fill(500, 0xff);
	ss::data htrep_comp = htrep.huffman_encode();
	ctx.log(std::format("encoded 500 length repeating character file, len={}", htrep_comp.size()));
	ss::data htrep_decomp = htrep_comp.huffman_decode();
	ctx.log(std::format("decoded 500 length repeating character file, len={} check {}", htrep_decomp.size(), (htrep_decomp == htrep)));

	for (const auto& l_file : std::filesystem::recursive_directory_iterator(".")) {
		if ((l_file.is_regular_file()) && (!(l_file.is_symlink()))) {
			ss::data ht;
			ht.load_file(l_file.path());
			//	ht.set_huffman_debug(true);
			ss::data ht_comp = ht.huffman_encode();
			//	ctx.log(ht_comp.as_hex_str());
			ss::data ht_decomp = ht_comp.huffman_decode();
			ctx.log(std::format("{} ht len: {} ht_comp len: {} ht_decomp len: {} check: {}", std::string(l_file.path()), ht.size(), ht_comp.size(), ht_decomp.size(), (ht_decomp == ht)));
		}
	}
	
	ctx.unregister_thread();
		ctx.log("thread should be missing");
	
	return 0;
}

