#include <iostream>
#include <string>
#include <format>
#include <filesystem>

#include "data.h"
#include "log.h"
#include "fs.h"

struct aesvec {
	std::uint64_t pt_l;
	std::uint64_t pt_r;
	std::uint64_t key_0;
	std::uint64_t key_1;
	std::uint64_t key_2;
	std::uint64_t key_3;
	std::uint64_t ct_l;
	std::uint64_t ct_r;
};

// a few popular AES 256 ECB test vectors

std::array<aesvec, 4> vecs = {{
	// plaintext                                    key                                                                                         ciphertext
	{ 0x6BC1BEE22E409F96ULL, 0xE93D7E117393172AULL, 0x603DEB1015CA71BEULL, 0x2B73AEF0857D7781ULL, 0x1F352C073B6108D7ULL, 0x2D9810A30914DFF4ULL, 0xF3EED1BDB5D2A03CULL, 0x064B5A7E3DB181F8ULL },
	{ 0xAE2D8A571E03AC9CULL, 0x9EB76FAC45AF8E51ULL, 0x603DEB1015CA71BEULL, 0x2B73AEF0857D7781ULL, 0x1F352C073B6108D7ULL, 0x2D9810A30914DFF4ULL, 0x591CCB10D410ED26ULL, 0xDC5BA74A31362870ULL },
	{ 0x30C81C46A35CE411ULL, 0xE5FBC1191A0A52EFULL, 0x603DEB1015CA71BEULL, 0x2B73AEF0857D7781ULL, 0x1F352C073B6108D7ULL, 0x2D9810A30914DFF4ULL, 0xB6ED21B99CA6F4F9ULL, 0xF153E7B1BEAFED1DULL },
	{ 0xF69F2445DF4F9B17ULL, 0xAD2B417BE66C3710ULL, 0x603DEB1015CA71BEULL, 0x2B73AEF0857D7781ULL, 0x1F352C073B6108D7ULL, 0x2D9810A30914DFF4ULL, 0x23304B7A39F9F3FFULL, 0x067D8D8F9E24ECC7ULL }
}};

struct ivvec {
	std::uint64_t iv_l;
	std::uint64_t iv_r;
	std::uint64_t ct_l;
	std::uint64_t ct_r;
};

// CBC mode vectors (assumes the plaintext and key used in the array above)

std::array<ivvec, 4> ivvecs = {{
	// IV                                           // expected ciphertext using IV in CBC mode
	{ 0x0001020304050607ULL, 0x08090A0B0C0D0E0FULL, 0xf58c4c04d6e5f1baULL, 0x779eabfb5f7bfbd6ULL },
	{ 0xF58C4C04D6E5F1BAULL, 0x779EABFB5F7BFBD6ULL, 0x9cfc4e967edb808dULL, 0x679f777bc6702c7dULL },
	{ 0x9CFC4E967EDB808DULL, 0x679F777BC6702C7DULL, 0x39f23369a9d9bacfULL, 0xa530e26304231461ULL },
	{ 0x39F23369A9D9BACFULL, 0xA530E26304231461ULL, 0xb2eb05e2c39be9fcULL, 0xda6c19078c6a9d1bULL }
}};

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
	
	ss::data aes_key1 = ss::data::aes256_key_random();
	ctx.log(std::format("random aes256 key: {}", aes_key1.as_hex_str_nospace()));
	ss::data aes_key2 = ss::data::aes256_key_schedule("Stephen Sviatko");
	ctx.log(std::format("scheduled aes256 key: {}", aes_key2.as_hex_str_nospace()));
	ss::data aes_key3 = ss::data::aes256_key_schedule("Stephen Sviatko");
	ctx.log(std::format("scheduled aes256 key: {}", aes_key3.as_hex_str_nospace()));

	ss::data aes_iv1 = ss::data::aes256_iv_random();
	ctx.log(std::format("random aes256 iv: {}", aes_iv1.as_hex_str_nospace()));
	ss::data aes_iv2 = ss::data::aes256_iv_schedule("Stephen Sviatko");
	ctx.log(std::format("scheduled aes256 iv: {}", aes_iv2.as_hex_str_nospace()));
	ss::data aes_iv3 = ss::data::aes256_iv_schedule("Stephen Sviatko");
	ctx.log(std::format("scheduled aes256 iv: {}", aes_iv3.as_hex_str_nospace()));

	for (std::size_t i = 0; i < vecs.size(); ++i) {
		ss::data aes_block;
		aes_block.set_network_byte_order(true);
		aes_block.write_uint64(vecs[i].pt_l);
		aes_block.write_uint64(vecs[i].pt_r);
		ss::data aes_key;
		aes_key.set_network_byte_order(true);
		aes_key.write_uint64(vecs[i].key_0);
		aes_key.write_uint64(vecs[i].key_1);
		aes_key.write_uint64(vecs[i].key_2);
		aes_key.write_uint64(vecs[i].key_3);
		ss::data expected_cipher;
		expected_cipher.set_network_byte_order(true);
		expected_cipher.write_uint64(vecs[i].ct_l);
		expected_cipher.write_uint64(vecs[i].ct_r);
		ss::data aes_block_enc = ss::data::aes256_block_encrypt(aes_block, aes_key);
		ctx.log(std::format("aes block {} key {} enc {} expected {} check {}", aes_block.as_hex_str_nospace(), aes_key.as_hex_str_nospace(), aes_block_enc.as_hex_str_nospace(), expected_cipher.as_hex_str_nospace(), (aes_block_enc == expected_cipher)));
		ss::data aes_block_dec = ss::data::aes256_block_decrypt(aes_block_enc, aes_key);
		ctx.log(std::format(" --> dec {} expected {} check {}", aes_block_dec.as_hex_str_nospace(), aes_block.as_hex_str_nospace(), (aes_block_dec == aes_block)));
	}
	
	// CBC test
	for (std::size_t i = 0; i < ivvecs.size(); ++i) {
		ss::data aes_data;
		aes_data.set_network_byte_order(true);
		aes_data.write_uint64(vecs[i].pt_l);
		aes_data.write_uint64(vecs[i].pt_r);
		ss::data aes_key;
		aes_key.set_network_byte_order(true);
		aes_key.write_uint64(vecs[i].key_0);
		aes_key.write_uint64(vecs[i].key_1);
		aes_key.write_uint64(vecs[i].key_2);
		aes_key.write_uint64(vecs[i].key_3);
		ss::data aes_iv;
		aes_iv.set_network_byte_order(true);
		aes_iv.write_uint64(ivvecs[i].iv_l);
		aes_iv.write_uint64(ivvecs[i].iv_r);
		ss::data expected_cipher;
		expected_cipher.set_network_byte_order(true);
		expected_cipher.write_uint64(ivvecs[i].ct_l);
		expected_cipher.write_uint64(ivvecs[i].ct_r);
		ss::data aes_data_enc = ss::data::aes256_encrypt_with_cbc(aes_data, aes_key, aes_iv);
		ctx.log(std::format("aes cbc data {} key {} enc {} expected {} check {}", aes_data.as_hex_str_nospace(), aes_key.as_hex_str_nospace(), aes_data_enc.as_hex_str_nospace(), expected_cipher.as_hex_str_nospace(), (aes_data_enc == expected_cipher)));
		ss::data aes_data_dec = ss::data::aes256_decrypt_with_cbc(aes_data_enc, aes_key, aes_iv);
		ctx.log(std::format(" --> dec {} expected {} check {}", aes_data_dec.as_hex_str_nospace(), aes_data.as_hex_str_nospace(), (aes_data_dec == aes_data)));
	}
	
	ss::data cbc_data;
	cbc_data.write_std_str("I think this is a good example of a text string to try to encode with AES. It is long enough.");
	cbc_data.write_int16(0);
	cbc_data.write_uint32(0xdeadbeef);
	ss::data aes256cbchmacsha2256_enc = ss::data::encrypt_aes256_cbc_hmac_sha2_256(cbc_data, aes_key1, aes_iv1);
	ctx.log(std::format("aes256-cbc-hmac-sha2-256 Full test: encrypted data is {} length {}", aes256cbchmacsha2256_enc.as_hex_str_nospace(), aes256cbchmacsha2256_enc.size()));
	ss::data aes256cbchmacsha2256_dec = ss::data::decrypt_aes256_cbc_hmac_sha2_256(aes256cbchmacsha2256_enc, aes_key1, aes_iv1);
	ctx.log(std::format("aes256-cbc-hmac-sha2-256 Full test: decrypted data is {} length {}", aes256cbchmacsha2256_dec.as_hex_str_nospace(), aes256cbchmacsha2256_dec.size()));
	ctx.log(std::format("check {}", cbc_data == aes256cbchmacsha2256_dec));
	
	return 0;
}
