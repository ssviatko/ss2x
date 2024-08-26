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
	/*
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
	*/
// CBC stuff
	
	ss::data cbc_data;
	cbc_data.write_std_str("7654321 Now is the time for ");
	cbc_data.write_uint8(0);
	ctx.log(std::format("CBC test: data is {} length {}", cbc_data.as_hex_str_nospace(), cbc_data.size()));
	ss::data cbc_key = ss::data::bf7_key_schedule("Stephen Sviatko");
	ctx.log(std::format("CBC test: key is {} length {}", cbc_key.as_hex_str_nospace(), cbc_key.size()));
	ss::data cbc_iv = ss::data::bf7_iv_schedule("Stephen Sviatko");
	ctx.log(std::format("CBC test: iv is {} length {}", cbc_iv.as_hex_str_nospace(), cbc_iv.size()));
	ss::data enc_cbcenc = ss::data::bf7_encrypt_with_cbc(cbc_data, cbc_key, cbc_iv);
	ctx.log(std::format("CBC test: encrypted data is {} length {}", enc_cbcenc.as_hex_str_nospace(), enc_cbcenc.size()));
	ss::data enc_cbcdec = ss::data::bf7_decrypt_with_cbc(enc_cbcenc, cbc_key, cbc_iv);
//	ctx.log(std::format("CBC test: key is {} length {}", cbc_key.as_hex_str_nospace(), cbc_key.size()));
//	ctx.log(std::format("CBC test: iv is {} length {}", cbc_iv.as_hex_str_nospace(), cbc_iv.size()));
	ctx.log(std::format("CBC test: decrypted data is {} length {}", enc_cbcdec.as_hex_str_nospace(), enc_cbcdec.size()));
	
	ss::data bf7cbchmacsha2256_enc = ss::data::encrypt_bf7_cbc_hmac_sha2_256(cbc_data, cbc_key, cbc_iv);
	ctx.log(std::format("Full test: encrypted data is {} length {}", bf7cbchmacsha2256_enc.as_hex_str_nospace(), bf7cbchmacsha2256_enc.size()));
	ss::data bf7cbchmacsha2256_dec = ss::data::decrypt_bf7_cbc_hmac_sha2_256(bf7cbchmacsha2256_enc, cbc_key, cbc_iv);
	ctx.log(std::format("Full test: decrypted data is {} length {}", bf7cbchmacsha2256_dec.as_hex_str_nospace(), bf7cbchmacsha2256_dec.size()));
	
	std::string l_lsmsg = "Please don't eat the Tide Pods. They are not good for you. Eat some nightshade berries instead, they are tasty and nutritious!";
	ctx.log(std::format("LS mesg: {}", l_lsmsg));
	std::string l_lsmsg_enc = ss::data::encode_little_secret("Stephen Sviatko", l_lsmsg);
	ctx.log(std::format("encoded: {}", l_lsmsg_enc));
	std::string l_lsmsg_dec = ss::data::decode_little_secret("Stephen Sviatko", l_lsmsg_enc);
	ctx.log(std::format("decoded: {}", l_lsmsg_dec));
	
	// deliberately generate error
	std::string l_lsmsg2 = "Artichokes three for a dollar, and if you give him another dollar, Artie will choke another three people.";
	ctx.log(std::format("LS mesg2 {}", l_lsmsg2));
	std::string l_lsmsg2_enc = ss::data::encode_little_secret("Stephen Sviatko", l_lsmsg2);
	ctx.log(std::format("encoded: {}", l_lsmsg2_enc));
	std::string l_lsmsg2_dec = ss::data::decode_little_secret("Stephenx Sviatko", l_lsmsg2_enc);
	ctx.log(std::format("decoded: {}", l_lsmsg2_dec));
	// with correct passphrase
	std::string l_lsmsg2_dec2 = ss::data::decode_little_secret("Stephen Sviatko", l_lsmsg2_enc);
	ctx.log(std::format("decoded: {}", l_lsmsg2_dec2));
	
	return 0;
}