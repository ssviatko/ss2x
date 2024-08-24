#include <iostream>
#include <string>
#include <format>

#include "data.h"
#include "log.h"
#include "fs.h"

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

int main(int argc, char **argv)
{
	std::cout << "blowfish framework test" << std::endl;
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
	return 0;
}
