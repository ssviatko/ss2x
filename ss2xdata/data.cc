#include "data.h"

namespace ss {

/* data_exception */

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

/* bit cursor */

void data::bit_cursor::set_absolute(std::uint64_t a_absolute)
{
	byte = a_absolute >> 3;
	bit = 7 - (a_absolute & 0x7);
}

std::uint64_t data::bit_cursor::get_absolute()
{
	std::uint64_t l_ret = byte << 3;
	l_ret |= (7 - bit);
	return l_ret;
}

/* data */

data::data()
: m_network_byte_order(false)
, m_read_cursor(0)
, m_write_cursor(0)
, m_delimiter(0xa)
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

void data::write_std_str(const std::string& a_str)
{
	std::vector<std::uint8_t> l_pass;
	l_pass.assign(a_str.c_str(), a_str.c_str() + a_str.size());
	write_raw_data(l_pass);
}

std::string data::read_std_str(std::size_t a_length)
{
	std::vector<std::uint8_t> l_read = read_raw_data(a_length);
	std::string l_ret((char *)(l_read.data()), l_read.size());
	return l_ret;
}

void data::write_std_str_delim(const std::string& a_str)
{
	write_std_str(a_str);
	write_uint8(m_delimiter);
}

std::optional<std::string> data::read_std_str_delim()
{
	std::vector<std::uint8_t>::iterator l_begin = m_buffer.begin();
	std::advance(l_begin, m_read_cursor);
	auto l_delim_pos = std::find(l_begin, m_buffer.end(), m_delimiter);
	if (l_delim_pos == m_buffer.end()) {
		return std::nullopt;
	} else {
		std::vector<std::uint8_t> l_work;
		std::copy(l_begin, l_delim_pos, std::back_inserter(l_work));
		std::string l_ret((char *)(l_work.data()), l_work.size());
		m_read_cursor += l_work.size() + 1;
		return l_ret;
	}
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

void data::random(std::size_t a_num_bytes)
{
	std::vector<std::uint8_t> l_pass(a_num_bytes);
	std::fill(l_pass.begin(), l_pass.end(), 0x00);
	std::random_device l_rd;
	std::mt19937 l_re(l_rd());
	std::uniform_int_distribution<int> l_dist(0, 255);
	for (auto& i : l_pass) {
		i = l_dist(l_re);
	}
	write_raw_data(l_pass);
}

void data::clear()
{
	m_buffer.clear();
	m_read_cursor = 0;
	m_write_cursor = 0;
	bit_cursor l_clear;
	m_read_bit_cursor = l_clear;
	m_write_bit_cursor = l_clear;
}

void data::truncate_back(std::size_t a_new_len)
{
	// do nothing if our truncation length is greater than the size of the buffer
	if (a_new_len >= m_buffer.size())
		return;
		
	m_buffer.resize(a_new_len);
	// adjust cursors if necessary
	if (m_write_cursor > a_new_len)
		m_write_cursor = a_new_len;
	if (m_read_cursor > a_new_len)
		m_read_cursor = a_new_len;
	if (m_write_bit_cursor.byte > a_new_len) {
		m_write_bit_cursor.byte = a_new_len;
		m_write_bit_cursor.bit = 7;
	}
	if (m_read_bit_cursor.byte > a_new_len) {
		m_read_bit_cursor.byte = a_new_len;
		m_read_bit_cursor.bit = 7;
	}
}

void data::assign(std::uint8_t *a_buffer, std::size_t a_len)
{
	for (size_t i = 0; i < a_len; ++i) {
		write_uint8(a_buffer[i]);
	}
}

bool data::compare(const data& a_data) const
{
	// differ in size? return false
	if (size() != a_data.size())
		return false;
		
	return (m_buffer == a_data.m_buffer);
}
	
/* operators */

bool data::operator==(const data& rhs) const
{
	return compare(rhs);
}

bool data::operator!=(const data& rhs) const
{
	return !compare(rhs);
}

/* bits */

void data::set_read_bit_cursor(bit_cursor a_bit_cursor)
{
	if (a_bit_cursor.byte < m_buffer.size()) {
		m_read_bit_cursor = a_bit_cursor;
	} else {
		if ((a_bit_cursor.byte == m_buffer.size()) && (a_bit_cursor.bit == 7)) {
			m_read_bit_cursor = a_bit_cursor;
		} else {
			data_exception e("attempt to set read bit cursor past end of buffer.");
			throw (e);
		}
	}
}

void data::set_write_bit_cursor(bit_cursor a_bit_cursor)
{
	if (a_bit_cursor.byte < m_buffer.size()) {
		m_write_bit_cursor = a_bit_cursor;
	} else {
		if ((a_bit_cursor.byte == m_buffer.size()) && (a_bit_cursor.bit == 7)) {
			m_write_bit_cursor = a_bit_cursor;
		} else {
			data_exception e("attempt to set write bit cursor past end of buffer.");
			throw (e);
		}
	}
}

void data::set_write_bit_cursor_to_append()
{
	bit_cursor l_temp;
	l_temp.byte = m_buffer.size();
	l_temp.bit = 7;
	set_write_bit_cursor(l_temp);
}

void data::write_true()
{
	write_bit(true);
}

void data::write_false()
{
	write_bit(false);
}

const std::uint8_t data::byte_mask[] = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };

void data::write_bit(bool a_bit)
{
	// make masks
	std::uint8_t l_and = byte_mask[m_write_bit_cursor.bit];
	std::uint8_t l_or = l_and ^ 0xff;

	// if write position is more than one byte past the buffer size, panic!
	// this should never, ever happen. This is protected against by the set_write_bit_cursor method above.
	if (m_write_bit_cursor.byte >= m_buffer.size() + 1) {
		data_exception e("write_bit: bit cursor set to impossible value.");
		throw (e);
	}
	
	// if write position is exactly one byte past the buffer size, add a byte to our buffer
	if (m_write_bit_cursor.byte == m_buffer.size()) {
		m_buffer.push_back(0x00);
	}
	
	// write bit at cursor position
	m_buffer[m_write_bit_cursor.byte] &= l_and;
	if (a_bit)
		m_buffer[m_write_bit_cursor.byte] |= l_or;

	// advance the write cursor
	if (m_write_bit_cursor.bit > 0) {
		--m_write_bit_cursor.bit;
	} else {
		++m_write_bit_cursor.byte;
		m_write_bit_cursor.bit = 7;
	}
}

void data::write_bit(std::uint64_t a_bit)
{
	a_bit ? write_true() : write_false();
}

void data::write_bits(std::uint64_t a_bits, std::uint16_t a_count)
{
	// sanity check our bit count
	if (!((a_count <= 64) && (a_count >= 0))) {
		data_exception e("write_bits: Bit count must between 0-64.");
		throw (e);
	}

	// shift everything over all the way to the left
	a_bits <<= (64 - a_count);

	// feed them into write_bit one at a time.
	// Note: A logical improvement to this routine would involve shifting the bits over
	// within a 5 byte frame to overlay the next bit write position, so the 3 inner bytes can be copied
	// directly and the two outer bytes can be and/ored. This temporary solution may prove to be fast
	// enough however.

	while (a_count > 0) {
		write_bit((a_bits & 0x8000000000000000ULL) > 0);
		a_bits <<= 1;
		--a_count;
	}
}

bool data::read_bit()
{
	// if this read will push us past the end of the buffer, throw an exception
	if (m_read_bit_cursor.byte >= m_buffer.size()) {
		data_exception e("read_bit: Attempt cursor-mode read past end of buffer.");
		throw(e);
	}

	std::uint8_t l_mask = byte_mask[m_read_bit_cursor.bit] ^ 0xff;
	std::uint8_t l_byte = m_buffer[m_read_bit_cursor.byte] & l_mask;

	// advance the read cursor
	if (m_read_bit_cursor.bit > 0) {
		--m_read_bit_cursor.bit;
	} else {
		++m_read_bit_cursor.byte;
		m_read_bit_cursor.bit = 7;
	}

	return (l_byte > 0); // true, if the bit we requested is set
}

std::uint64_t data::read_bits(std::uint16_t a_count)
{
	// sanity check our bit count
	if (!((a_count <= 64) && (a_count >= 0))) {
		data_exception e("ss::data::read_bits: Bit count must between 0-64.");
		throw (e);
	}

	std::uint64_t l_ret = 0;

	while (a_count > 0) {
		l_ret <<= 1;
		if (read_bit())
			l_ret |= 0x0000000000000001ULL;
		--a_count;
	}
	return l_ret;
}

std::string data::as_bits()
{
	std::string l_ret;
	
	// does not disturb bit cursors
	for (std::uint8_t& i : m_buffer) {
		std::uint8_t l_work = i;
		for (int j = 0; j <= 7; ++j) {
			if (l_work & 0x80) {
				l_ret += '1';
			} else {
				l_ret += '0';
			}
			l_work <<= 1;
		}
		l_ret += " "; // justify bytes
	}
	return l_ret;
}

void data::from_bits(const std::string& a_str)
{
	clear();
	for (const auto& i : a_str) {
		if (i == '0')
			write_false();
		if (i == '1')
			write_true();
		// ... and all other characters ignored. Feel free to use spaces or other delimiters.
	}
}

/* cursor management */

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

/* private routines for textual presentation and initialization */

std::string data::hex_str(const std::uint8_t *a_data, std::size_t a_len)
{
	std::stringstream ss;

	ss << std::hex;
	for (std::size_t i = 0; i < a_len; ++i)
		ss << std::setw(2) << std::setfill('0') << (int)a_data[i];
	return ss.str();
}

std::uint8_t *data::hex_decode(const std::string a_str, std::size_t *decode_len)
{
	// bail out if we're not justified on a 2 character boundary
	if ((a_str.size() % 2) != 0) {
		data_exception e("hex_decode: String must be a multiple of 2 characters.");
		throw(e);
	}

	std::size_t l_len = a_str.size() / 2;
	*decode_len = l_len;
//	std::cout << "hex_decode: l_len=" << l_len << " a_str.size=" << a_str.size() << std::endl;
	std::uint8_t *l_dec = new std::uint8_t[l_len];

	for (std::size_t i = 0; i < a_str.size(); i += 2) {
		std::uint8_t l_in[2], l_out = 0;
		l_in[0] = a_str[i];
		l_in[1] = a_str[i + 1];
		for (int j = 0; j < 2; ++j) {
			if ((l_in[j] >= 'A') && (l_in[j] <= 'F'))
				l_in[j] -= ('A' - 10);
			else if ((l_in[j] >= 'a') && (l_in[j] <= 'f'))
				l_in[j] -= ('a' - 10);
			else if ((l_in[j] >= '0') && (l_in[j] <= '9'))
				l_in[j] -= '0';
			else {
				// illegal char in string
				delete[] l_dec;
				data_exception e("hex_decode: Illegal character in string.");
				throw(e);
			}
		}
		l_out = ((l_in[0] & 0x0f) << 4);
		l_out |= l_in[1] & 0x0f;
		l_dec[i / 2] = l_out;
	}

	return l_dec;
}

std::string data::base64_str(const std::uint8_t *a_data, std::size_t a_len)
{
	std::stringstream ss;
	std::uint8_t l_temp[3], l_out[5];
	std::uint8_t l_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	for (std::size_t i = 0; i < a_len; i += 3) {
		int l_numbytes = (i + 3 < a_len) ? 3 : a_len - i;
//              std::cout << "l_numbytes=" << l_numbytes << std::endl;
		memset(l_temp, 0, 3);
		memcpy(l_temp, a_data + i, l_numbytes);
		l_out[0] = l_chars[(l_temp[0] & 0xfc) >> 2];
		l_out[1] = l_chars[((l_temp[0] & 0x03) << 4) | ((l_temp[1] & 0xf0) >> 4)];
		l_out[2] = l_chars[((l_temp[1] & 0x0f) << 2) | ((l_temp[2] & 0xc0) >> 6)];
		l_out[3] = l_chars[l_temp[2] & 0x3f];
		l_out[4] = '\0';
		if (l_numbytes < 3)
			l_out[3] = '=';
		if (l_numbytes == 1)
			l_out[2] = '=';
		ss << l_out;
	}

	return ss.str();
}

std::uint8_t *data::base64_decode(const std::string a_str, std::size_t *decode_len)
{
	// bail out if we're not justified on a 4 character boundary
       if ((a_str.size() % 4) != 0) {
		data_exception e("base64_decode: String must be a multiple of 4 characters.");
		throw(e);
	}

	std::uint32_t l_len = (a_str.size() * 3 / 4);
	*decode_len = l_len;
//      std::cout << "base64_decode: l_len=" << l_len << " a_str.size=" << a_str.size() << std::endl;
    std::uint8_t *l_dec = new std::uint8_t[l_len];

	for (std::size_t i = 0, io = 0; i < a_str.size(); i += 4, io += 3) {
		std::uint8_t l_in[4], l_out[3];
		memset(l_out, 0, 3);
		for (int j = 0; j < 4; ++j)
			l_in[j] = a_str[i + j];
		if (l_in[3] == '=') {
			l_in[3] = 'A'; // zero it out
			(*decode_len)--;
		}
		if (a_str[i + 2] == '=') {
			l_in[2] = 'A';
			(*decode_len)--;
		}
//              std::cout << "i=" << i << " decode_len=" << *decode_len << std::endl;
		for (int j = 0; j < 4; ++j) {
			if ((l_in[j] >= 'A') && (l_in[j] <= 'Z'))
				l_in[j] -= 'A';
			else if ((l_in[j] >= 'a') && (l_in[j] <= 'z'))
				l_in[j] = l_in[j] - 'a' + 26;
			else if ((l_in[j] >= '0') && (l_in[j] <= '9'))
				l_in[j] = l_in[j] - '0' + 52;
			else if (l_in[j] == '+')
				l_in[j] = 62;
			else if (l_in[j] == '/')
				l_in[j] = 63;
			else {
				// illegal char in string
				delete[] l_dec;
				data_exception e("ss::data::base64_decode: Illegal character in string.");
				throw(e);
			}
		}
		l_out[0] = (l_in[0] << 2 | l_in[1] >> 4);
		l_out[1] = (l_in[1] << 4 | l_in[2] >> 2);
		l_out[2] = (((l_in[2] << 6) & 0xc0) | l_in[3]);
		memcpy(l_dec + io, l_out, 3);
	}

	return l_dec;
}

// textual presentation and initialization

void data::write_base64(const std::string& a_str)
{
	std::size_t l_decode_len;
	std::uint8_t *l_buff = base64_decode(a_str, &l_decode_len);
//	std::cout << "l_decode_len=" << l_decode_len << std::endl;
	std::vector<std::uint8_t> l_pass;
	l_pass.assign(l_buff, l_buff + l_decode_len);
	write_raw_data(l_pass);
	delete[] l_buff;
}

std::string data::read_base64(std::size_t a_len)
{
	std::vector<std::uint8_t> l_work = read_raw_data(a_len);
	std::string ret = base64_str(l_work.data(), l_work.size());
	return ret;
}

std::string data::as_base64()
{
	// return entire buffer as base64, without disturbing the cursors
	std::string ret = base64_str(m_buffer.data(), m_buffer.size());
	return ret;
}

void data::from_base64(const std::string& a_str)
{
	// clear the buffer and write entre contents from provided base64 string
	clear();
	write_base64(a_str);
}

void data::write_hex_str(const std::string& a_str)
{
	std::size_t l_decode_len;
	std::uint8_t *l_buff = hex_decode(a_str, &l_decode_len);
//	std::cout << "l_decode_len=" << l_decode_len << std::endl;
	std::vector<std::uint8_t> l_pass;
	l_pass.assign(l_buff, l_buff + l_decode_len);
	write_raw_data(l_pass);
	delete[] l_buff;
}

std::string data::read_hex_str(std::size_t a_len)
{
	std::vector<std::uint8_t> l_work = read_raw_data(a_len);
	std::string ret = hex_str(l_work.data(), l_work.size());

	return ret;
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

/* encryption */

data data::bf_key_random()
{
	data l_ret;
	
	l_ret.random(56); // random 448 bit Blowfish key
	return l_ret;
}

data data::bf_key_schedule(const std::string& a_string)
{
	// schedule a key by hashing a string
	data l_work;
	l_work.write_std_str(a_string);
	data l_hash = l_work.sha2_512();
	l_hash.truncate_back(56);
	return l_hash;
}

data data::bf_block_encrypt(data& a_block, data& a_key)
{
	// enforce size constraints on block and key
	if (a_block.size() != 8) {
		data_exception e("Blowfish block must be 8 bytes in length.");
		throw (e);
	}
	if ((a_key.size() < 8) || (a_key.size() > 56)) {
		data_exception e("Blowfish key must be between 8 and 56 bytes in length.");
		throw (e);
	}
	ss::bf::block l_work(a_block.m_buffer.data(), a_key.m_buffer.data(), a_key.size());
	l_work.encrypt();
	data l_ret;
	const std::uint8_t *blockdata = l_work.get_blockdata();
	for (int i = 0; i < 8; ++i)
		l_ret.write_uint8(blockdata[i]);
	return l_ret;
}

data data::bf_block_decrypt(data& a_block, data& a_key)
{
	// enforce size constraints on block and key
	if (a_block.size() != 8) {
		data_exception e("Blowfish block must be 8 bytes in length.");
		throw (e);
	}
	if ((a_key.size() < 8) || (a_key.size() > 56)) {
		data_exception e("Blowfish key must be between 8 and 56 bytes in length.");
		throw (e);
	}
	ss::bf::block l_work(a_block.m_buffer.data(), a_key.m_buffer.data(), a_key.size());
	l_work.decrypt();
	data l_ret;
	const std::uint8_t *blockdata = l_work.get_blockdata();
	for (int i = 0; i < 8; ++i)
		l_ret.write_uint8(blockdata[i]);
	return l_ret;
}

};

