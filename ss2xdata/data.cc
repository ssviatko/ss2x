#include "data.h"

namespace ss {

data_exception::data_exception(const char* a_what) throw()
{
	m_what = a_what;
}

data_exception::data_exception(const std::string& a_what) throw()
{
	m_what = a_what;
}

const char* data_exception::what() const throw()
{
	return m_what.c_str();
}

data::data()
: m_network_byte_order(false)
, m_read_cursor(0)
, m_write_cursor(0)
{
}

data::~data()
{
}

void data::dump_hex()
{
	std::cout << as_hex_str() << std::endl;
}

std::string data::as_hex_str()
{
	std::stringstream l_ss;
	for (auto& i : m_buffer)
		l_ss << std::hex << std::setfill('0') << std::setw(2) << (int)i << " ";
	return l_ss.str();
}

std::string data::as_hex_str_nospace()
{
	std::stringstream l_ss;
	for (auto& i : m_buffer)
		l_ss << std::hex << std::setfill('0') << std::setw(2) << (int)i;
	return l_ss.str();
}

void data::save_file(const std::string& a_filename)
{
	std::ofstream l_savefile;
	l_savefile.open(a_filename.c_str(), std::ios::binary | std::ios::trunc);
	if (!l_savefile.is_open()) {
		data_exception e("unable to open file to save data.");
		throw(e);
	}

	l_savefile.write((char *)m_buffer.data(), m_buffer.size());
	if (!l_savefile.good()) {
		data_exception e("unable to write to file.");
		throw(e);
	}

	l_savefile.close();
}

void data::load_file(const std::string& a_filename)
{
	std::ifstream l_loadfile;
	l_loadfile.open(a_filename.c_str(), std::ios::binary);
	if (!l_loadfile.is_open()) {
		data_exception e("unable to open file to load data.");
		throw(e);
	}

	std::array<char, 4096> l_buff;

	do {
		l_loadfile.read(l_buff.data(), 4096);
		if (l_loadfile.bad()) {
			data_exception e("unable to read file.");
			throw(e);
		}
		std::uint64_t l_bytes_read = l_loadfile.gcount();
		std::vector<std::uint8_t> l_pass;
		l_pass.assign(l_buff.data(), l_buff.data() + l_bytes_read);
		write_raw_data(l_pass);
	} while (!l_loadfile.eof());

	l_loadfile.close();
}

std::vector<std::uint8_t> data::read_raw_data(std::size_t a_num_bytes)
{
	if (m_read_cursor + a_num_bytes > m_buffer.size()) {
		data_exception e("attempt to read past end of buffer.");
		throw (e);
	}
	
	std::vector<std::uint8_t> l_ret;
	std::vector<std::uint8_t>::iterator l_begin = m_buffer.begin();
	std::advance(l_begin, m_read_cursor);
	std::vector<std::uint8_t>::iterator l_end = l_begin;
	std::advance(l_end, a_num_bytes);
	std::copy(l_begin, l_end, std::back_inserter(l_ret));
	m_read_cursor += a_num_bytes;
	return l_ret;
}

void data::write_raw_data(std::vector<std::uint8_t>& a_vector)
{
	// add space to vector if we're overwriting the end
	if (m_write_cursor + a_vector.size() > m_buffer.size()) {
		// empty vector of appropriate size
//		std::cout << "adding " << ((m_write_cursor + a_vector.size()) - m_buffer.size()) << " elements to m_buffer." << std::endl;
		std::vector<std::uint8_t> l_empty((m_write_cursor + a_vector.size()) - m_buffer.size());
		m_buffer.insert(m_buffer.end(), l_empty.begin(), l_empty.end());
	}
	std::vector<std::uint8_t>::iterator l_dest = m_buffer.begin();
	std::advance(l_dest, m_write_cursor);
//	std::cout << "copy to m_write_cursor " << m_write_cursor << std::endl;
	std::copy(a_vector.begin(), a_vector.end(), l_dest);
	m_write_cursor += a_vector.size();
}

void data::fill(std::size_t a_num_bytes, std::uint8_t a_val)
{
	std::vector<std::uint8_t> l_pass(a_num_bytes);
	std::fill(l_pass.begin(), l_pass.end(), a_val);
	write_raw_data(l_pass);
}

void data::clear()
{
	m_buffer.clear();
	m_read_cursor = 0;
	m_write_cursor = 0;
}

void data::set_write_cursor(std::size_t a_write_cursor)
{
	if (a_write_cursor > m_buffer.size()) {
		data_exception e("attempt to set write cursor past end of buffer.");
		throw (e);
	}
	m_write_cursor = a_write_cursor;
}

void data::set_write_cursor_to_append()
{
	m_write_cursor = m_buffer.size();
}

void data::set_read_cursor(std::size_t a_read_cursor)
{
	if (a_read_cursor > m_buffer.size()) {
		data_exception e("attempt to set read cursor past end of buffer.");
		throw (e);
	}
	m_read_cursor = a_read_cursor;
}

// 8

void data::write_uint8(std::uint8_t a_uint8)
{
	std::vector<std::uint8_t> l_pass;
	l_pass.push_back(a_uint8);
	write_raw_data(l_pass);
}

std::uint8_t data::read_uint8()
{
	std::vector<std::uint8_t> l_read = read_raw_data(sizeof(std::uint8_t));
	size1_union l_ret;
	l_ret.uint8_val = l_read[0];
	return l_ret.uint8_val;
}

void data::write_int8(std::int8_t a_int8)
{
	std::vector<std::uint8_t> l_pass;
	size1_union l_work;
	l_work.int8_val = a_int8;
	l_pass.push_back(l_work.uint8_val);
	write_raw_data(l_pass);
}

std::int8_t data::read_int8()
{
	std::vector<std::uint8_t> l_read = read_raw_data(sizeof(std::uint8_t));
	size1_union l_ret;
	l_ret.uint8_val = l_read[0];
	return l_ret.int8_val;
}

// 16

void data::write_uint16(std::uint16_t a_uint16)
{
	std::vector<std::uint8_t> l_pass;
	size2_union l_work;
	l_work.uint16_val = a_uint16;
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.uint16_val = std::byteswap(l_work.uint16_val);
	}
	l_pass.assign(l_work.raw, l_work.raw + 2);
	write_raw_data(l_pass);
}

std::uint16_t data::read_uint16()
{
	std::vector<std::uint8_t> l_read = read_raw_data(sizeof(std::uint16_t));
	size2_union l_ret;
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.uint16_val = std::byteswap(l_ret.uint16_val);
	}
	return l_ret.uint16_val;
}

void data::write_int16(std::int16_t a_int16)
{
	std::vector<std::uint8_t> l_pass;
	size2_union l_work;
	l_work.int16_val = a_int16;
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.int16_val = std::byteswap(l_work.int16_val);
	}
	l_pass.assign(l_work.raw, l_work.raw + 2);
	write_raw_data(l_pass);
}

std::int16_t data::read_int16()
{
	std::vector<std::uint8_t> l_read = read_raw_data(sizeof(std::int16_t));
	size2_union l_ret;
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.int16_val = std::byteswap(l_ret.int16_val);
	}
	return l_ret.int16_val;
}

// 32

void data::write_uint32(std::uint32_t a_uint32)
{
	std::vector<std::uint8_t> l_pass;
	size4_union l_work;
	l_work.uint32_val = a_uint32;
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.uint32_val = std::byteswap(l_work.uint32_val);
	}
	l_pass.assign(l_work.raw, l_work.raw + 4);
	write_raw_data(l_pass);
}

std::uint32_t data::read_uint32()
{
	std::vector<std::uint8_t> l_read = read_raw_data(sizeof(std::uint32_t));
	size4_union l_ret;
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.uint32_val = std::byteswap(l_ret.uint32_val);
	}
	return l_ret.uint32_val;
}

void data::write_int32(std::int32_t a_int32)
{
	std::vector<std::uint8_t> l_pass;
	size4_union l_work;
	l_work.int32_val = a_int32;
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.int32_val = std::byteswap(l_work.int32_val);
	}
	l_pass.assign(l_work.raw, l_work.raw + 4);
	write_raw_data(l_pass);
}

std::int32_t data::read_int32()
{
	std::vector<std::uint8_t> l_read = read_raw_data(sizeof(std::int32_t));
	size4_union l_ret;
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.int32_val = std::byteswap(l_ret.int32_val);
	}
	return l_ret.int32_val;
}

// 64

void data::write_uint64(std::uint64_t a_uint64)
{
	std::vector<std::uint8_t> l_pass;
	size8_union l_work;
	l_work.uint64_val = a_uint64;
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.uint64_val = std::byteswap(l_work.uint64_val);
	}
	l_pass.assign(l_work.raw, l_work.raw + 8);
	write_raw_data(l_pass);
}

std::uint64_t data::read_uint64()
{
	std::vector<std::uint8_t> l_read = read_raw_data(sizeof(std::uint64_t));
	size8_union l_ret;
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.uint64_val = std::byteswap(l_ret.uint64_val);
	}
	return l_ret.uint64_val;
}

void data::write_int64(std::int64_t a_int64)
{
	std::vector<std::uint8_t> l_pass;
	size8_union l_work;
	l_work.int64_val = a_int64;
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.int64_val = std::byteswap(l_work.int64_val);
	}
	l_pass.assign(l_work.raw, l_work.raw + 8);
	write_raw_data(l_pass);
}

std::int64_t data::read_int64()
{
	std::vector<std::uint8_t> l_read = read_raw_data(sizeof(std::int64_t));
	size8_union l_ret;
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.int64_val = std::byteswap(l_ret.int64_val);
	}
	return l_ret.int64_val;
}

// floats

void data::write_float(float a_float)
{
	std::vector<std::uint8_t> l_pass;
	size4_union l_work;
	l_work.float_val = a_float;
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		// note: std::byteswap doesn't appear to work with float
		l_work.uint32_val = std::byteswap(l_work.uint32_val);
	}
	l_pass.assign(l_work.raw, l_work.raw + 4);
	write_raw_data(l_pass);
}

float data::read_float()
{
	std::vector<std::uint8_t> l_read = read_raw_data(sizeof(float));
	size4_union l_ret;
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.uint32_val = std::byteswap(l_ret.uint32_val);
	}
	return l_ret.float_val;
}

void data::write_double(double a_double)
{
	std::vector<std::uint8_t> l_pass;
	size8_union l_work;
	l_work.double_val = a_double;
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		// note: std::byteswap doesn't appear to work with float
		l_work.uint64_val = std::byteswap(l_work.uint64_val);
	}
	l_pass.assign(l_work.raw, l_work.raw + 8);
	write_raw_data(l_pass);
}

double data::read_double()
{
	std::vector<std::uint8_t> l_read = read_raw_data(sizeof(double));
	size8_union l_ret;
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.uint64_val = std::byteswap(l_ret.uint64_val);
	}
	return l_ret.double_val;
}

void data::write_longdouble(long double a_longdouble)
{
	std::vector<std::uint8_t> l_pass;
	size16_union l_work;
	l_work.longdouble_val = a_longdouble;
	l_pass.assign(l_work.raw, l_work.raw + 16);
	write_raw_data(l_pass);
}

long double data::read_longdouble()
{
	std::vector<std::uint8_t> l_read = read_raw_data(sizeof(long double));
	size16_union l_ret;
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	return l_ret.longdouble_val;
}

// hashing

const std::uint32_t data::crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

std::uint32_t data::crc32(std::uint32_t a_crc)
{
	const std::uint8_t *p;
	std::size_t size = m_buffer.size();

	p = m_buffer.data();
	a_crc = a_crc ^ ~0U;

	while (size--)
		a_crc = crc32_tab[(a_crc ^ *p++) & 0xFF] ^ (a_crc >> 8);

	return a_crc ^ ~0U;
}

data data::md5()
{
	data l_digest;
	l_digest.fill(16, 0); // empty space to hold digest
	MD5_CTX l_ctx;
	MD5Init(&l_ctx);
	MD5Update(&l_ctx, (unsigned char *)m_buffer.data(), m_buffer.size());
	MD5Final(l_digest.m_buffer.data(), &l_ctx);
	l_digest.m_read_cursor = 0;
	l_digest.m_write_cursor = 0;
	return l_digest;
}

data data::sha1()
{
	data l_digest;
	SHA1Context l_ctx;
	SHA1Reset(&l_ctx);
	SHA1Input(&l_ctx, (unsigned char *)m_buffer.data(), m_buffer.size());
	SHA1Result(&l_ctx);
	l_digest.set_network_byte_order(true);
	l_digest.write_uint32(l_ctx.Message_Digest[0]);
	l_digest.write_uint32(l_ctx.Message_Digest[1]);
	l_digest.write_uint32(l_ctx.Message_Digest[2]);
	l_digest.write_uint32(l_ctx.Message_Digest[3]);
	l_digest.write_uint32(l_ctx.Message_Digest[4]);
	l_digest.set_read_cursor(0);
	l_digest.set_write_cursor(0);
	return l_digest;
}

data data::sha2_224()
{
	data l_digest;
	l_digest.fill(28, 0); // empty space to hold digest
	sha224_ctx l_ctx;
	sha224_init(&l_ctx);
	sha224_update(&l_ctx, (const uint8_t *)m_buffer.data(), m_buffer.size());
	sha224_final(&l_ctx, l_digest.m_buffer.data());
	l_digest.set_read_cursor(0);
	l_digest.set_write_cursor(0);
	return l_digest;
}

data data::sha2_256()
{
	data l_digest;
	l_digest.fill(32, 0); // empty space to hold digest
	sha256_ctx l_ctx;
	sha256_init(&l_ctx);
	sha256_update(&l_ctx, (const uint8_t *)m_buffer.data(), m_buffer.size());
	sha256_final(&l_ctx, l_digest.m_buffer.data());
	l_digest.set_read_cursor(0);
	l_digest.set_write_cursor(0);
	return l_digest;
}

data data::sha2_384()
{
	data l_digest;
	l_digest.fill(48, 0); // empty space to hold digest
	sha384_ctx l_ctx;
	sha384_init(&l_ctx);
	sha384_update(&l_ctx, (const uint8_t *)m_buffer.data(), m_buffer.size());
	sha384_final(&l_ctx, l_digest.m_buffer.data());
	l_digest.set_read_cursor(0);
	l_digest.set_write_cursor(0);
	return l_digest;
}

data data::sha2_512()
{
	data l_digest;
	l_digest.fill(64, 0); // empty space to hold digest
	sha512_ctx l_ctx;
	sha512_init(&l_ctx);
	sha512_update(&l_ctx, (const uint8_t *)m_buffer.data(), m_buffer.size());
	sha512_final(&l_ctx, l_digest.m_buffer.data());
	l_digest.set_read_cursor(0);
	l_digest.set_write_cursor(0);
	return l_digest;
}

};


