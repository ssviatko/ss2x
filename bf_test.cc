#include <iostream>
#include <string>
#include <format>
#include <filesystem>

#include "data.h"
#include "log.h"
#include "fs.h"

typedef struct {
	std::uint64_t key;
	std::uint64_t clear;
	std::uint64_t enc;
} bf_block_test_vector;

std::array<bf_block_test_vector, 37> bf_vec = {{
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
	{ 0x3849674C2602319EULL, 0x51454B582DDF440AULL, 0xA25E7856CF2651EBULL },
	{ 0x04B915BA43FEB5B6ULL, 0x42FD443059577FA2ULL, 0x353882B109CE8F1AULL },
	{ 0x0113B970FD34F2CEULL, 0x059B5E0851CF143AULL, 0x48F4D0884C379918ULL },
	{ 0x0170F175468FB5E6ULL, 0x0756D8E0774761D2ULL, 0x432193B78951FC98ULL },
	{ 0x43297FAD38E373FEULL, 0x762514B829BF486AULL, 0x13F04154D69D1AE5ULL },
	{ 0x07A7137045DA2A16ULL, 0x3BDD119049372802ULL, 0x2EEDDA93FFD39C79ULL },
	{ 0x04689104C2FD3B2FULL, 0x26955F6835AF609AULL, 0xD887E0393C2DA6E3ULL },
	{ 0x37D06BB516CB7546ULL, 0x164D5E404F275232ULL, 0x5F99D04F5B163969ULL },
	{ 0x1F08260D1AC2465EULL, 0x6B056E18759F5CCAULL, 0x4A057A3B24D3977BULL },
	{ 0x584023641ABA6176ULL, 0x004BD6EF09176062ULL, 0x452031C1E4FADA8EULL },
	{ 0x025816164629B007ULL, 0x480D39006EE762F2ULL, 0x7555AE39F59B87BDULL },
	{ 0x49793EBC79B3258FULL, 0x437540C8698F3CFAULL, 0x53C55F9CB49FC019ULL },
	{ 0x4FB05E1515AB73A7ULL, 0x072D43A077075292ULL, 0x7A8E7BFA937E89A3ULL },
	{ 0x49E95D6D4CA229BFULL, 0x02FE55778117F12AULL, 0xCF9C5D7A4986ADB5ULL },
	{ 0x018310DC409B26D6ULL, 0x1D9D5C5018F728C2ULL, 0xD1ABB290658BC778ULL },
	{ 0x1C587F1C13924FEFULL, 0x305532286D6F295AULL, 0x55CB3774D13EF201ULL },
	{ 0x0101010101010101ULL, 0x0123456789ABCDEFULL, 0xFA34EC4847B268B2ULL },
	{ 0x1F1F1F1F0E0E0E0EULL, 0x0123456789ABCDEFULL, 0xA790795108EA3CAEULL },
	{ 0xE0FEE0FEF1FEF1FEULL, 0x0123456789ABCDEFULL, 0xC39E072D9FAC631DULL },
	{ 0x0000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL, 0x014933E0CDAFF6E4ULL },
	{ 0xFFFFFFFFFFFFFFFFULL, 0x0000000000000000ULL, 0xF21E9A77B71C49BCULL },
	{ 0x0123456789ABCDEFULL, 0x0000000000000000ULL, 0x245946885754369AULL },
	{ 0xFEDCBA9876543210ULL, 0xFFFFFFFFFFFFFFFFULL, 0x6B5C5A9C5D9E0A5AULL }
}};

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
	
	// CBC stuff
	ss::data enc_randiv = ss::data::bf_iv_random();
	ctx.log(std::format("BF: random 64 bit iv: {}", enc_randiv.as_hex_str_nospace()));
	ss::data enc_schediv = ss::data::bf_iv_schedule("Stephen Sviatko");
	ctx.log(std::format("BF: scheduled 64 bit iv: {}", enc_schediv.as_hex_str_nospace()));
	ss::data enc_schediv2 = ss::data::bf_iv_schedule("Stephen Sviatko");
	ctx.log(std::format("BF: scheduled 64 bit iv2: {}", enc_schediv2.as_hex_str_nospace()));
	
	ss::data cbc_data;
	cbc_data.write_std_str("7654321 Now is the time for ");
	cbc_data.write_uint8(0);
	ctx.log(std::format("CBC test: data is {} length {}", cbc_data.as_hex_str_nospace(), cbc_data.size()));
	ss::data cbc_key;
	cbc_key.write_hex_str("0123456789ABCDEFF0E1D2C3B4A59687");
	ctx.log(std::format("CBC test: key is {} length {}", cbc_key.as_hex_str_nospace(), cbc_key.size()));
	ss::data cbc_iv;
	cbc_iv.write_hex_str("FEDCBA9876543210");
	ctx.log(std::format("CBC test: iv is {} length {}", cbc_iv.as_hex_str_nospace(), cbc_iv.size()));
	ss::data enc_cbcenc = ss::data::bf_encrypt_with_cbc(cbc_data, cbc_key, cbc_iv);
	ctx.log(std::format("CBC test: encrypted data is {} length {}", enc_cbcenc.as_hex_str_nospace(), enc_cbcenc.size()));
	ss::data enc_cbcdec = ss::data::bf_decrypt_with_cbc(enc_cbcenc, cbc_key, cbc_iv);
	ctx.log(std::format("CBC test: key is {} length {}", cbc_key.as_hex_str_nospace(), cbc_key.size()));
	ctx.log(std::format("CBC test: iv is {} length {}", cbc_iv.as_hex_str_nospace(), cbc_iv.size()));
	ctx.log(std::format("CBC test: decrypted data is {} length {}", enc_cbcdec.as_hex_str_nospace(), enc_cbcdec.size()));
	
	ss::data bfcbchmacsha2256_enc = ss::data::encrypt_bf_cbc_hmac_sha2_256(cbc_data, cbc_key, cbc_iv);
	ctx.log(std::format("Full test: encrypted data is {} length {}", bfcbchmacsha2256_enc.as_hex_str_nospace(), bfcbchmacsha2256_enc.size()));
	ss::data bfcbchmacsha2256_dec = ss::data::decrypt_bf_cbc_hmac_sha2_256(bfcbchmacsha2256_enc, cbc_key, cbc_iv);
	ctx.log(std::format("Full test: decrypted data is {} length {}", bfcbchmacsha2256_dec.as_hex_str_nospace(), bfcbchmacsha2256_dec.size()));

	// test file encryption
	for (const auto& l_file : std::filesystem::recursive_directory_iterator(".")) {
		if ((l_file.is_regular_file()) && (!(l_file.is_symlink()))) {
			ss::data ht;
			ht.load_file(l_file.path());
			ss::data ht_key = ss::data::bf_key_random();
			ss::data ht_iv = ss::data::bf_iv_random();
			ss::data ht_enc = ss::data::encrypt_bf_cbc_hmac_sha2_256(ht, ht_key, ht_iv);
			ss::data ht_dec = ss::data::decrypt_bf_cbc_hmac_sha2_256(ht_enc, ht_key, ht_iv);
			ctx.log(std::format("{} ht len: {} ht_enc len: {} ht_dec len: {} check: {}", std::string(l_file.path()), ht.size(), ht_enc.size(), ht_dec.size(), (ht_dec == ht)));
		}
	}
	return 0;
}
