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
, m_circular_mode(false)
, m_read_cursor(0)
, m_write_cursor(0)
, m_delimiter(0xa)
, m_huffman_debug(false)
{
}

data::~data()
{
}

data::data(const data& a_data)
{
	copy_construct(a_data);
}

data::data(data&& a_data)
{
	copy_construct(a_data);
//	// clear foreign data object after we move it
//	a_data.clear();
}

void data::copy_construct(const data& a_data)
{
	m_network_byte_order = a_data.m_network_byte_order;
	m_circular_mode = a_data.m_circular_mode;
	m_read_cursor = a_data.m_read_cursor;
	m_write_cursor = a_data.m_write_cursor;
	m_delimiter = a_data.m_delimiter;
	m_huffman_debug = a_data.m_huffman_debug;
	m_read_bit_cursor = a_data.m_read_bit_cursor;
	m_write_bit_cursor = a_data.m_write_bit_cursor;
	m_buffer = a_data.m_buffer;
}

void data::dump_hex() const
{
	std::cout << as_hex_str() << std::endl;
}

std::string data::as_hex_str() const
{
	std::stringstream l_ss;
	for (const auto i : m_buffer) {
		l_ss << std::hex << std::setfill('0') << std::setw(2) << (int)i << " ";
	}
	return l_ss.str();
}

std::string data::as_hex_str_nospace() const
{
	std::stringstream l_ss;
	for (const auto i : m_buffer) {
		l_ss << std::hex << std::setfill('0') << std::setw(2) << (int)i;
	}
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
	if (!m_circular_mode)
		std::advance(l_begin, m_read_cursor);
	auto l_delim_pos = std::find(l_begin, m_buffer.end(), m_delimiter);
	if (l_delim_pos == m_buffer.end()) {
		return std::nullopt;
	} else {
		std::vector<std::uint8_t> l_work;
		std::copy(l_begin, l_delim_pos, std::back_inserter(l_work));
		std::string l_ret((char *)(l_work.data()), l_work.size());
		if (m_circular_mode)
			truncate_front(l_work.size() + 1);
		else
			m_read_cursor += l_work.size() + 1;
		return l_ret;
	}
}

std::vector<std::uint8_t> data::read_raw_data(std::size_t a_num_bytes)
{
	std::vector<std::uint8_t> l_ret;
	if (m_circular_mode) {
		// if number of bytes requested exceeds buffer size, error
		if (a_num_bytes > m_buffer.size()) {
			data_exception e("attempt circular mode read larger than buffer size.");
			throw (e);
		}
		std::vector<std::uint8_t>::iterator l_final = m_buffer.begin();
		std::advance(l_final, a_num_bytes);
		std::copy(m_buffer.begin(), l_final, std::back_inserter(l_ret));
		truncate_front(a_num_bytes);
	} else {
		// normal mode read
		if (m_read_cursor + a_num_bytes > m_buffer.size()) {
			data_exception e("attempt to read past end of buffer.");
			throw (e);
		}
		std::vector<std::uint8_t>::iterator l_begin = m_buffer.begin();
		std::advance(l_begin, m_read_cursor);
		std::vector<std::uint8_t>::iterator l_end = l_begin;
		std::advance(l_end, a_num_bytes);
		std::copy(l_begin, l_end, std::back_inserter(l_ret));
		m_read_cursor += a_num_bytes;
	}
	return l_ret;
}

void data::write_raw_data(const std::vector<std::uint8_t>& a_vector)
{
	if (m_circular_mode)
		set_write_cursor_to_append();
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

void data::truncate_front(std::size_t a_trunc_len)
{
	// if truncation length exceeds size of buffer, this is an error!
	if (a_trunc_len > m_buffer.size()) {
		data_exception e("truncate_front: truncation length exceeds buffer size.");
		throw (e);
	}
	std::vector<std::uint8_t>::iterator l_new_front = m_buffer.begin();
	std::advance(l_new_front, a_trunc_len);
	m_buffer.erase(m_buffer.begin(), l_new_front);
	// correct cursors
	if (m_read_cursor <= a_trunc_len)
		m_read_cursor = 0;
	else
		m_read_cursor -= a_trunc_len;
	if (m_write_cursor <= a_trunc_len)
		m_write_cursor = 0;
	else
		m_write_cursor -= a_trunc_len;
	// leave the bit cursors alone - beware! Don't use bit read/write routines in circular mode.
}

void data::assign(const std::uint8_t *a_buffer, std::size_t a_len)
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

void data::append_data(const data& a_data)
{
	set_write_cursor_to_append();
	write_raw_data(a_data.m_buffer);
}

std::size_t data::size() const
{
	std::int64_t l_size = m_buffer.size();
	if (l_size < 0) {
//		data_exception e("got <0 for buffer size");
//		throw (e);
//		std::cerr << "data::size got <0!" << std::endl;
		return 0;
	} else {
		return l_size;
	}
}

/* operators */

bool data::operator==(const data& rhs) const
{
	return ((*this <=> rhs) == std::strong_ordering::equivalent);
}

bool data::operator!=(const data& rhs) const
{
	return !((*this <=> rhs) == std::strong_ordering::equivalent);
}

std::strong_ordering data::operator<=>(const data& rhs) const
{
	// comparison; iterate buffer(s) and return as soon as condition is met
	// if both buffers are empty, they are equal, so return equivalent
	if ((this->size() == 0) && (rhs.size() == 0))
		return std::strong_ordering::equivalent;
	// if this contains data and rhs is empty, this is > rhs
	if ((this->size() > 0) && (rhs.size() == 0))
		return std::strong_ordering::greater;
	// if this is empty and rhs contains data, then this is < rhs
	if ((this->size() == 0) && (rhs.size() > 0))
		return std::strong_ordering::less;
	// both buffers contain data, compare them byte by byte. if they are stil
	// equal after comparing the smaller number of bytes, then the larger object
	// wins as the greater value.
	std::size_t l_min = std::min(this->size(), rhs.size());
	for (std::size_t i = 0; i < l_min; ++i) {
		// first one to have a lesser or greater value wins
		if (this->m_buffer[i] < rhs.m_buffer[i]) {
			return std::strong_ordering::less;
		}
		if (this->m_buffer[i] > rhs.m_buffer[i]) {
			return std::strong_ordering::greater;
		}
	}
	// if we made it here, the first n common bytes of the buffers are equal. Test remaining size
	if (this->size() < rhs.size())
		return std::strong_ordering::less;
	if (this->size() > rhs.size())
		return std::strong_ordering::greater;
	// fall through here. buffers are identical in size and contain identical data.
	return std::strong_ordering::equivalent;
}

data& data::operator=(const data& a_data)
{
	if (this != &a_data) {
		copy_construct(a_data);
	} else {
		data_exception e("data operator=: Self assignment detected.");
		throw(e);
	}
	return *this;
}

data& data::operator=(data&& a_data)
{
	if (this != &a_data) {
		copy_construct(a_data);
//		// since we're moving, clear the foreign data object
//		a_data.clear();
	} else {
		data_exception e("data operator=: Self assignment detected.");
		throw(e);
	}
	return *this;
}

data& data::operator+=(const data& a_data)
{
	append_data(a_data);
	return *this;
}

std::uint8_t& data::operator[](std::size_t index)
{
	return m_buffer.at(index);
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

// specializations

void data::write_uint24(std::uint32_t a_uint32)
{
	std::vector<std::uint8_t> l_pass;
	size4_union l_work;
	l_work.uint32_val = a_uint32;
	if (l_work.uint32_val > 16777215) {
		data_exception e("uint24 value should not exceed 16777215.");
		throw (e);
	}
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.uint32_val = std::byteswap(l_work.uint32_val);
	}
	if ((std::endian::native == std::endian::big) || m_network_byte_order) {
		l_pass.assign(l_work.raw + 1, l_work.raw + 4);
	} else {
		l_pass.assign(l_work.raw, l_work.raw + 3);
	}
	write_raw_data(l_pass);	
}

std::uint32_t data::read_uint24()
{
	std::vector<std::uint8_t> l_read = read_raw_data(3);
	size4_union l_ret;
	l_ret.uint32_val = 0;
	if (m_network_byte_order) {
		l_read.insert(l_read.begin(), 0x00);
	} else {
		l_read.push_back(0x00);
	}
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.uint32_val = std::byteswap(l_ret.uint32_val);
	}
	return l_ret.uint32_val;	
}

void data::write_int24(std::int32_t a_int32)
{
	std::vector<std::uint8_t> l_pass;
	size4_union l_work;
	l_work.int32_val = a_int32;
	if (l_work.int32_val > 8388607) {
		data_exception e("uint24 value should not exceed 8388607.");
		throw (e);
	}
	if (l_work.int32_val < -8388608) {
		data_exception e("uint24 value should not be less than -8388608.");
		throw (e);
	}
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.int32_val = std::byteswap(l_work.int32_val);
	}
	if ((std::endian::native == std::endian::big) || m_network_byte_order) {
		l_pass.assign(l_work.raw + 1, l_work.raw + 4);
	} else {
		l_pass.assign(l_work.raw, l_work.raw + 3);
	}
	write_raw_data(l_pass);		
}

std::int32_t data::read_int24()
{
	std::vector<std::uint8_t> l_read = read_raw_data(3);
	size4_union l_ret;
	l_ret.int32_val = 0;
	if (m_network_byte_order) {
		if ((l_read[0] & 0x80) > 0) // sign extension
			l_read.insert(l_read.begin(), 0xff);
		else
			l_read.insert(l_read.begin(), 0x00);
	} else {
		if ((l_read[2] & 0x80) > 0)
			l_read.push_back(0xff);
		else
			l_read.push_back(0x00);
	}
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.int32_val = std::byteswap(l_ret.int32_val);
	}
	return l_ret.int32_val;	
}

// 40 bit integers

void data::write_uint40(std::uint64_t a_uint64)
{
	std::vector<std::uint8_t> l_pass;
	size8_union l_work;
	l_work.uint64_val = a_uint64;
	if (l_work.uint64_val > 1099511627775ULL) {
		data_exception e("uint24 value should not exceed 1099511627775.");
		throw (e);
	}
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.uint64_val = std::byteswap(l_work.uint64_val);
	}
	if ((std::endian::native == std::endian::big) || m_network_byte_order) {
		l_pass.assign(l_work.raw + 3, l_work.raw + 8);
	} else {
		l_pass.assign(l_work.raw, l_work.raw + 5);
	}
	write_raw_data(l_pass);	
}

std::uint64_t data::read_uint40()
{
	std::vector<std::uint8_t> l_read = read_raw_data(5);
	size8_union l_ret;
	l_ret.uint64_val = 0;
	if (m_network_byte_order) {
		l_read.insert(l_read.begin(), 0x00);
		l_read.insert(l_read.begin(), 0x00);
		l_read.insert(l_read.begin(), 0x00);
	} else {
		l_read.push_back(0x00);
		l_read.push_back(0x00);
		l_read.push_back(0x00);
	}
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.uint64_val = std::byteswap(l_ret.uint64_val);
	}
	return l_ret.uint64_val;	
}

void data::write_int40(std::int64_t a_int64)
{
	std::vector<std::uint8_t> l_pass;
	size8_union l_work;
	l_work.int64_val = a_int64;
	if (l_work.int64_val > 549755813887LL) {
		data_exception e("uint24 value should not exceed 549755813887.");
		throw (e);
	}
	if (l_work.int64_val < -549755813888LL) {
		data_exception e("uint24 value should not be less than -549755813888.");
		throw (e);
	}
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.int64_val = std::byteswap(l_work.int64_val);
	}
	if ((std::endian::native == std::endian::big) || m_network_byte_order) {
		l_pass.assign(l_work.raw + 3, l_work.raw + 8);
	} else {
		l_pass.assign(l_work.raw, l_work.raw + 5);
	}
	write_raw_data(l_pass);		
}

std::int64_t data::read_int40()
{
	std::vector<std::uint8_t> l_read = read_raw_data(5);
	size8_union l_ret;
	l_ret.int64_val = 0;
	if (m_network_byte_order) {
		if ((l_read[0] & 0x80) > 0) { // sign extension
			l_read.insert(l_read.begin(), 0xff);
			l_read.insert(l_read.begin(), 0xff);
			l_read.insert(l_read.begin(), 0xff);
		} else {
			l_read.insert(l_read.begin(), 0x00);
			l_read.insert(l_read.begin(), 0x00);
			l_read.insert(l_read.begin(), 0x00);
		}
	} else {
		if ((l_read[4] & 0x80) > 0) {
			l_read.push_back(0xff);
			l_read.push_back(0xff);
			l_read.push_back(0xff);
		} else {
			l_read.push_back(0x00);
			l_read.push_back(0x00);
			l_read.push_back(0x00);
		}
	}
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.int64_val = std::byteswap(l_ret.int64_val);
	}
	return l_ret.int64_val;	
}

// 48 bit integers

void data::write_uint48(std::uint64_t a_uint64)
{
	std::vector<std::uint8_t> l_pass;
	size8_union l_work;
	l_work.uint64_val = a_uint64;
	if (l_work.uint64_val > 281474976710655ULL) {
		data_exception e("uint24 value should not exceed 281474976710655.");
		throw (e);
	}
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.uint64_val = std::byteswap(l_work.uint64_val);
	}
	if ((std::endian::native == std::endian::big) || m_network_byte_order) {
		l_pass.assign(l_work.raw + 2, l_work.raw + 8);
	} else {
		l_pass.assign(l_work.raw, l_work.raw + 6);
	}
	write_raw_data(l_pass);	
}

std::uint64_t data::read_uint48()
{
	std::vector<std::uint8_t> l_read = read_raw_data(6);
	size8_union l_ret;
	l_ret.uint64_val = 0;
	if (m_network_byte_order) {
		l_read.insert(l_read.begin(), 0x00);
		l_read.insert(l_read.begin(), 0x00);
	} else {
		l_read.push_back(0x00);
		l_read.push_back(0x00);
	}
	std::copy(l_read.begin(), l_read.end(), l_ret.raw);
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_ret.uint64_val = std::byteswap(l_ret.uint64_val);
	}
	return l_ret.uint64_val;		
}

void data::write_int48(std::int64_t a_int64)
{
	std::vector<std::uint8_t> l_pass;
	size8_union l_work;
	l_work.int64_val = a_int64;
	if (l_work.int64_val > 140737488355327LL) {
		data_exception e("uint24 value should not exceed 140737488355327.");
		throw (e);
	}
	if (l_work.int64_val < -140737488355328LL) {
		data_exception e("uint24 value should not be less than -140737488355328.");
		throw (e);
	}
	if ((std::endian::native == std::endian::little) && m_network_byte_order) {
		l_work.int64_val = std::byteswap(l_work.int64_val);
	}
	if ((std::endian::native == std::endian::big) || m_network_byte_order) {
		l_pass.assign(l_work.raw + 2, l_work.raw + 8);
	} else {
		l_pass.assign(l_work.raw, l_work.raw + 6);
	}
	write_raw_data(l_pass);		
}

std::int64_t data::read_int48()
{
	std::vector<std::uint8_t> l_read = read_raw_data(6);
	size8_union l_ret;
	l_ret.int64_val = 0;
	if (m_network_byte_order) {
		if ((l_read[0] & 0x80) > 0) { // sign extension
			l_read.insert(l_read.begin(), 0xff);
			l_read.insert(l_read.begin(), 0xff);
		} else {
			l_read.insert(l_read.begin(), 0x00);
			l_read.insert(l_read.begin(), 0x00);
		}
	} else {
		if ((l_read[4] & 0x80) > 0) {
			l_read.push_back(0xff);
			l_read.push_back(0xff);
		} else {
			l_read.push_back(0x00);
			l_read.push_back(0x00);
		}
	}
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
	// zero out bytes 10-15 of the long double (not used anyway)
	// to shut up Valgrind's uninitialized value error
	for (std::size_t i = 10; i < 16; ++i)
		l_work.raw[i] = 0;
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

data data::bf7_key_random()
{
	data l_ret;
	
	l_ret.random(392); // random 3136 bit Blowfish7 key
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

data data::bf7_key_schedule(const std::string& a_string)
{
	data l_work1;
	l_work1.write_std_str(a_string);
	data l_hash1 = l_work1.sha2_512();
	data l_hash2 = l_hash1.sha2_512();
	data l_hash3 = l_hash2.sha2_512();
	data l_hash4 = l_hash3.sha2_512();
	data l_hash5 = l_hash4.sha2_512();
	data l_hash6 = l_hash5.sha2_512();
	data l_hash7 = l_hash6.sha2_512();
	l_hash1 += l_hash2;
	l_hash1 += l_hash3;
	l_hash1 += l_hash4;
	l_hash1 += l_hash5;
	l_hash1 += l_hash6;
	l_hash1 += l_hash7;
	l_hash1.truncate_back(392);
	return l_hash1;
}

data data::bf_iv_random()
{
	data l_ret;
	
	l_ret.random(8); // random 64 bit Blowfish iv
	return l_ret;
}

data data::bf7_iv_random()
{
	data l_ret;
	
	l_ret.random(16); // random 128 bit Blowfish7 iv
	return l_ret;
}

data data::bf_iv_schedule(const std::string& a_string)
{
	// schedule a key by hashing a string
	data l_work;
	l_work.write_std_str(a_string);
	data l_hash = l_work.sha2_384();
	l_hash.truncate_back(8);
	return l_hash;
}

data data::bf7_iv_schedule(const std::string& a_string)
{
	data l_work;
	l_work.write_std_str(a_string);
	data l_hash = l_work.sha2_384();
	l_hash.truncate_back(16);
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

data data::bf7_block_encrypt(data& a_block, data& a_key)
{
	// enforce size constraints on block and key
	if (a_block.size() != 16) {
		data_exception e("Blowfish7 block must be 16 bytes in length.");
		throw (e);
	}
	if (a_key.size() != 392) {
		data_exception e("Blowfish7 key must be exactly 392 bytes in length.");
		throw (e);
	}
	
	// divide key into 7 keys
	std::vector<data> keys;
	ss::data l_empty;
	a_key.set_read_cursor(0);
	for (std::size_t i = 0; i < 7; ++i) {
		keys.push_back(l_empty);
		for (std::size_t j = 0; j < 56; ++j) {
			keys[i].write_uint8(a_key.read_uint8());
		}
	}
	
	// debug: print our partial keys
	for (std::size_t i = 0; i < 7; ++i) {
		std::cout << std::format("partial key {}: {}", i + 1, keys[i].as_hex_str_nospace()) << std::endl;
	}
	
	std::array<std::uint8_t, 20> l_work;
	auto show_work = [&]() {
		std::cout << "work buffer: ";
		for (std::size_t i = 0; i < 20; ++i) {
			std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)l_work[i];
			if (i == 15)
				std::cout << "-";
		}
		std::cout << std::endl;
	};
	
	memcpy(l_work.data(), a_block.buffer(), 16);
	// wrap around first four bytes on end
	memcpy(l_work.data() + 16, a_block.buffer(), 4);
	show_work();
	
	ss::bf::block block1((std::uint8_t *)l_work.data(), keys[0].buffer(), 56);
	block1.encrypt();
	memcpy(l_work.data(), block1.get_blockdata(), 8);
	memcpy(l_work.data() + 16, l_work.data(), 4);
	show_work();
	
	ss::bf::block block2((std::uint8_t *)l_work.data() + 4, keys[1].buffer(), 56);
	block2.encrypt();
	memcpy(l_work.data() + 4, block2.get_blockdata(), 8);
	show_work();
	
	ss::bf::block block3((std::uint8_t *)l_work.data() + 8, keys[2].buffer(), 56);
	block3.encrypt();
	memcpy(l_work.data() + 8, block3.get_blockdata(), 8);
	show_work();
	
	ss::bf::block block4((std::uint8_t *)l_work.data() + 12, keys[3].buffer(), 56);
	block4.encrypt();
	memcpy(l_work.data() + 12, block4.get_blockdata(), 8);
	memcpy(l_work.data(), l_work.data() + 16, 4);
	show_work();
	
	ss::bf::block block5((std::uint8_t *)l_work.data(), keys[4].buffer(), 56);
	block5.decrypt();
	memcpy(l_work.data(), block5.get_blockdata(), 8);
	show_work();
	
	ss::bf::block block6((std::uint8_t *)l_work.data() + 8, keys[5].buffer(), 56);
	block6.decrypt();
	memcpy(l_work.data() + 8, block6.get_blockdata(), 8);
	show_work();
	
	ss::bf::block block7a((std::uint8_t *)l_work.data(), keys[6].buffer(), 56);
	ss::bf::block block7b((std::uint8_t *)l_work.data() + 8, keys[6].buffer(), 56);
	block7a.encrypt();
	block7b.encrypt();
	memcpy(l_work.data(), block7a.get_blockdata(), 8);
	memcpy(l_work.data() + 8, block7b.get_blockdata(), 8);
	show_work();
	
	data l_ret;
	for (std::size_t i = 0; i < 16; ++i) {
		l_ret.write_uint8(l_work[i]);
	}
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

data data::bf7_block_decrypt(data& a_block, data& a_key)
{
	// enforce size constraints on block and key
	if (a_block.size() != 16) {
		data_exception e("Blowfish7 block must be 16 bytes in length.");
		throw (e);
	}
	if (a_key.size() != 392) {
		data_exception e("Blowfish7 key must be exactly 392 bytes in length.");
		throw (e);
	}
	
	// divide key into 7 keys
	std::vector<data> keys;
	ss::data l_empty;
	a_key.set_read_cursor(0);
	for (std::size_t i = 0; i < 7; ++i) {
		keys.push_back(l_empty);
		for (std::size_t j = 0; j < 56; ++j) {
			keys[i].write_uint8(a_key.read_uint8());
		}
	}
	
	// debug: print our partial keys
	for (std::size_t i = 0; i < 7; ++i) {
		std::cout << std::format("partial key {}: {}", i + 1, keys[i].as_hex_str_nospace()) << std::endl;
	}
	
	std::array<std::uint8_t, 20> l_work;
	auto show_work = [&]() {
		std::cout << "work buffer: ";
		for (std::size_t i = 0; i < 20; ++i) {
			std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)l_work[i];
			if (i == 15)
				std::cout << "-";
		}
		std::cout << std::endl;
	};
	
	memcpy(l_work.data(), a_block.buffer(), 16);
	memset(l_work.data() + 16, 0, 4);
	show_work();
	
	ss::bf::block block7a((std::uint8_t *)l_work.data(), keys[6].buffer(), 56);
	ss::bf::block block7b((std::uint8_t *)l_work.data() + 8, keys[6].buffer(), 56);
	block7a.decrypt();
	block7b.decrypt();
	memcpy(l_work.data(), block7a.get_blockdata(), 8);
	memcpy(l_work.data() + 8, block7b.get_blockdata(), 8);
	show_work();
	
	ss::bf::block block6((std::uint8_t *)l_work.data() + 8, keys[5].buffer(), 56);
	block6.encrypt();
	memcpy(l_work.data() + 8, block6.get_blockdata(), 8);
	show_work();
	
	ss::bf::block block5((std::uint8_t *)l_work.data(), keys[4].buffer(), 56);
	block5.encrypt();
	memcpy(l_work.data(), block5.get_blockdata(), 8);
	memcpy(l_work.data() + 16, l_work.data(), 4);
	show_work();
	
	ss::bf::block block4((std::uint8_t *)l_work.data() + 12, keys[3].buffer(), 56);
	block4.decrypt();
	memcpy(l_work.data() + 12, block4.get_blockdata(), 8);
	memcpy(l_work.data(), l_work.data() + 16, 4);
	show_work();
	
	ss::bf::block block3((std::uint8_t *)l_work.data() + 8, keys[2].buffer(), 56);
	block3.decrypt();
	memcpy(l_work.data() + 8, block3.get_blockdata(), 8);
	show_work();
	
	ss::bf::block block2((std::uint8_t *)l_work.data() + 4, keys[1].buffer(), 56);
	block2.decrypt();
	memcpy(l_work.data() + 4, block2.get_blockdata(), 8);
	show_work();
	
	ss::bf::block block1((std::uint8_t *)l_work.data(), keys[0].buffer(), 56);
	block1.decrypt();
	memcpy(l_work.data(), block1.get_blockdata(), 8);
	show_work();
	
	data l_ret;
	for (std::size_t i = 0; i < 16; ++i) {
		l_ret.write_uint8(l_work[i]);
	}
	return l_ret;
}

data data::bf_encrypt_with_cbc(data& a_data, data& a_key, data& a_iv)
{
	// sanity check IV
	if (a_iv.size() != 8) {
		throw data_exception("initialization vector needs to be same as block size.");
	}
	
	data l_ret;
	if (a_data.size() == 0) {
		// just return empty buffer if we were passed zero length data
		return l_ret;
	}
	
	ss::data l_block;
	l_block.fill(8, 0);
	ss::bf::block l_bf(l_block.buffer(), a_key.buffer(), a_key.size());
	l_bf.set_iv(a_iv.buffer());
	std::size_t l_pos = 0;

	auto block_clear = [&]() {
		l_block.set_write_cursor(0);
		for (std::size_t i = 0; i < 8; ++i)
			l_block.write_uint8(0);
		l_block.set_write_cursor(0);
	};
	
	do {
		std::size_t l_end = l_pos + 8;
		l_end = (l_end > a_data.size() ? a_data.size() : l_end);
//		std::cout << "reading from " << l_pos << " to one less than " << l_end << std::endl;
		block_clear();
		a_data.set_read_cursor(l_pos);
		for (std::size_t i = l_pos; i < l_end; ++i) {
			l_block.write_uint8(a_data.read_uint8());
		}
		l_bf.set_blockdata(l_block.buffer());
		l_bf.encrypt_with_cbc();
		block_clear();
		l_block.assign(l_bf.get_blockdata(), 8);
		l_ret += l_block;
		l_pos += 8;
	} while (l_pos < a_data.size());
	
	return l_ret;
}

data data::bf_decrypt_with_cbc(data& a_data, data& a_key, data& a_iv)
{
	// sanity check IV
	if (a_iv.size() != 8) {
		throw data_exception("initialization vector needs to be same as block size.");
	}
	
	data l_ret;
	// bail out if the input buffer isn't a multiple of 8 bytes
	if ((a_data.size() % 8) != 0) {
		throw data_exception("Input buffer must be justified on an 8 byte boundary.");
	}

	ss::data l_block;
	l_block.fill(8, 0);
	ss::bf::block l_bf(l_block.buffer(), a_key.buffer(), a_key.size());
	l_bf.set_iv(a_iv.buffer());
	std::size_t l_pos = 0;

	auto block_clear = [&]() {
		l_block.set_write_cursor(0);
		for (std::size_t i = 0; i < 8; ++i)
			l_block.write_uint8(0);
		l_block.set_write_cursor(0);
	};
		
	do {
		block_clear();
		a_data.set_read_cursor(l_pos);
		for (std::size_t i = 0; i < 8; ++i) {
			l_block.write_uint8(a_data.read_uint8());
		}
		l_bf.set_blockdata(l_block.buffer());
		l_bf.decrypt_with_cbc();
		block_clear();
		l_block.assign(l_bf.get_blockdata(), 8);
		l_ret += l_block;
		l_pos += 8;
	} while (l_pos < a_data.size());
	
	return l_ret;
}

data data::encrypt_bf_cbc_hmac_sha2_256(data& a_data, data& a_key, data& a_iv)
{
	std::array<std::uint8_t, 32> l_hmac; // space to hold hmac-sha256
	
	// compute the hmac on our plaintext and store it
	hmacsha256(a_key.buffer(), a_key.size(), a_data.buffer(), a_data.size(), l_hmac.data());

	// tack on terminator byte to our data
	data l_withterm = a_data;
	l_withterm.set_write_cursor_to_append();
	l_withterm.write_uint8(0x80);

	// create a new input buffer and seal the hmac value inside
	data l_temp;
	l_temp.assign(l_hmac.data(), 32);
	l_temp += l_withterm;
	data l_ret = data::bf_encrypt_with_cbc(l_temp, a_key, a_iv);
	return l_ret;
}

data data::decrypt_bf_cbc_hmac_sha2_256(data& a_data, data& a_key, data& a_iv)
{
	// sanity check m_input_buffer_len
	if (a_data.size() < 40) {
		// 32 byte hmac + at least one 8 byte block
		throw data_exception("Blowfish CBC HMAC/SHA256 buffer must contain at least 40 bytes to decrypt.");
	}

	std::array<std::uint8_t, 32> l_hmac; // space to hold hmac-sha256
	std::array<std::uint8_t, 32> l_computed; // space for computed hmac

	data l_dec = data::bf_decrypt_with_cbc(a_data, a_key, a_iv);
	

	// save bottom 32 bytes, which contain our hmac
	l_dec.set_read_cursor(0);
	for (auto& i : l_hmac)
		i = l_dec.read_uint8();
		
	// move everything down by 32
	l_dec.truncate_front(32);
	
	// backtrack until we find the terminator byte
	std::int64_t l_decsize = l_dec.size();
	bool l_found = false;
	while (!l_found) {
		l_decsize--;
		if (l_decsize < 0) {
			// searched the whole buffer, didn't find a terminator
			throw data_exception("Blowfish CBC HMAC/SHA256 buffer must contain terminator byte.");
		}
		if (l_dec[l_decsize] == 0x80) {
			l_dec.truncate_back(l_decsize);
			l_found = true;
		}
	}
	
	// make sure the saved hmac matches the computed hmac
//	std::cout << "key size " << a_key.size() << " l_dec.size " << l_dec.size() << std::endl;
	hmacsha256(a_key.buffer(), a_key.size(), l_dec.buffer(), l_dec.size(), l_computed.data());
	if (memcmp(l_computed.data(), l_hmac.data(), 32)) {
		throw data_exception("HMAC mismatch error on decrypt. Possible data corruption.");
	}
	return l_dec;
}

/* compression */
	
data data::huffman_encode() const
{
	std::deque<huff_tree_node> l_in;
	std::vector<huff_tree_node> l_out;
	std::int16_t l_out_root = -1;
	std::uint64_t l_freq[256];
	std::uint64_t l_max_freq = 0;

	// check for zero length edge case
	// just write a magic cookie with zero length
	if (m_buffer.size() == 0) {
		data l_encoded;
		bit_cursor l_write_bit_cursor;
		l_encoded.set_write_bit_cursor(l_write_bit_cursor);
		l_encoded.write_bits(HUFF_MAGIC_COOKIE, 32);
		l_encoded.write_bits(static_cast<std::uint64_t>(m_buffer.size()), 64);
		return l_encoded;
	}
	
	// populate frequency table
	for (std::uint64_t i = 0; i < 256; ++i)
		l_freq[i] = 0;
	for (std::size_t i = 0; i < m_buffer.size(); ++i)
		++l_freq[m_buffer[i]];

	// make a node for each symbol with >0 frequency
	for (std::uint64_t i = 0; i < 256; ++i) {
		if (l_freq[i] > 0) {
			if (l_freq[i] > l_max_freq)
				l_max_freq = l_freq[i];
			if (m_huffman_debug) std::cout << "huffman_encode: creating huff_tree_node for " << std::hex << i << std::dec << " freq=" << l_freq[i] << std::endl;
			huff_tree_node l_temp;
			l_temp.symbol = i;
			l_temp.freq = l_freq[i];
			l_in.push_back(l_temp);
		}
	}
	if (m_huffman_debug) std::cout << "huffman_encode: l_max_freq=" << l_max_freq << std::endl;

	// arrange in ascending order of frequency
	std::sort(l_in.begin(), l_in.end());

	if (m_huffman_debug) {
		for (std::deque<huff_tree_node>::iterator l_it = l_in.begin(); l_it != l_in.end(); ++l_it)
			std::cout << "huffman_encode: sym:" << std::hex << int(l_it->symbol) << std::dec << " freq:" << l_it->freq << std::endl;
	}
	std::int16_t l_next_id = 0;

	while (l_in.size() > 1) {
		// create new internal node to point to each pulled node
		huff_tree_node l_apex;
		l_apex.type = huff_tree_node::INTERNAL;

		// get the next 2 lowest frequency symbols and assign them id's
		huff_tree_node l_left = l_in.front();
		l_in.pop_front();
		l_left.id = l_next_id++;
		l_apex.left_id = l_left.id;
		huff_tree_node l_right = l_in.front();
		l_in.pop_front();
		l_right.id = l_next_id++;
		l_apex.right_id = l_right.id;

		// assign apex sum of children's frequencies and add it back on the end of l_in
		l_apex.freq = l_left.freq + l_right.freq;
		l_in.push_back(l_apex);
		std::sort(l_in.begin(), l_in.end());

		// stick the children in our result vector
		l_out.push_back(l_left);
		l_out.push_back(l_right);

		if (m_huffman_debug) {
			std::cout << "huffman_encode: Adding: L:" << l_left.type << "-" << int(l_left.symbol) << "/" << l_left.freq;
			std::cout << " IDs: L" << l_left.left_id << " A" << l_left.id << " R" << l_left.right_id;
			std::cout << " R:" << l_right.type << "-" << int(l_right.symbol) << "/" << l_right.freq;
			std::cout << " IDs: L" << l_right.left_id << " A" << l_right.id << " R" << l_right.right_id;
			std::cout << std::endl;
		}
	}

	// there should be one node left in l_in, and this is our root node.. unless it's a leaf, then cover that special case
	huff_tree_node l_root = l_in.front();
	l_in.pop_front();
	l_root.id = l_next_id++;
	// check if it's a leaf - this would indicate that it was the only node in the list
	// this edge case happens when there is only one symbol type, e.g. a file full of zeros or a single byte file
	// consequentially, give it an apex node and make that the root instead
	if (l_root.type == huff_tree_node::LEAF) {
		// l_apex becomes the root; l_root becomes the single child
		huff_tree_node l_apex;
		l_apex.type = huff_tree_node::INTERNAL;
		l_apex.id = l_next_id++;
		l_apex.freq = l_root.freq;
		l_apex.left_id = l_root.id;
		l_apex.right_id = -1;
		l_out.push_back(l_root);
		l_out.push_back(l_apex);
		l_out_root = l_apex.id;
		if (m_huffman_debug) {
			std::cout << "huffman_encode: 1-node edge case - Adding: Root:" << l_apex.type;
			std::cout << " IDs: L" << l_apex.left_id << " A" << l_apex.id << " R" << l_apex.right_id << std::endl;
			std::cout << "huffman_encode: Root node is " << l_out_root << std::endl;
			std::cout << "huffman_encode: single child node:" << l_root.type << "-" << int(l_root.symbol) << "/" << l_root.freq;
			std::cout << " IDs: L" << l_root.left_id << " A" << l_root.id << " R" << l_root.right_id << std::endl;
		}
	} else {
		l_out.push_back(l_root);
		l_out_root = l_root.id;
		if (m_huffman_debug) {
			std::cout << "huffman_encode: Adding: Root:" << l_root.type << "-" << int(l_root.symbol) << "/" << l_root.freq;
			std::cout << " IDs: L" << l_root.left_id << " A" << l_root.id << " R" << l_root.right_id << std::endl;
			std::cout << "huffman_encode: Root node is " << l_out_root << std::endl;
		}
	}

	// construct code table by traversing the tree
	std::map<std::uint8_t, std::pair<std::uint64_t, std::int16_t> > l_codes;
	std::uint64_t l_cur_code = 0;
	std::int16_t l_cur_code_len = 0;
	std::int16_t l_cur_node = l_out_root;
	std::stack<std::int16_t> l_parent;
	bool l_finished = false;
	bool l_traverse_up = false; // set when we are going backwards

	do {
		// get our current node
		huff_tree_node l_apex = l_out[l_cur_node];
		if (m_huffman_debug) std::cout << "huffman_encode: l_cur_node:" << l_cur_node << " l_traverse_up:" << std::boolalpha << l_traverse_up << std::noboolalpha << std::endl;
		// did we arrive here from a child node?
		if (l_traverse_up) {
			l_traverse_up = false;

			// remove last bool from l_cur_code
			bool l_last = l_cur_code & 0x1;
			l_cur_code >>= 1;
			--l_cur_code_len;
			if ((l_last == true) && (l_cur_node == l_out_root)) {
				// we're at the root node and we arrived here from the right, so we're done!
				l_finished = true;
				continue;
			}
			// check our single leave edge case
			if ((l_last == false) && (l_cur_node == l_out_root) && (l_apex.right_id == -1)) {
				l_finished = true;
				continue;
			}
			if (l_last == false) {
				// we arrived here from the left, so categorize the right hand node now
				if (l_apex.right_id > -1) {
					l_cur_code <<= 1;
					l_cur_code |= 0x1;
					++l_cur_code_len;
					l_parent.push(l_cur_node);
					l_cur_node = l_apex.right_id;
					continue;
				}
			} else {
				if (m_huffman_debug) std::cout << "arrived from the right" << std::endl;
				// arrived from right, but we're not at the root, so traverse up yet again
				l_cur_node = l_parent.top();
				l_parent.pop();
				l_traverse_up = true;
				continue;
			}
		}

		// if we're a leaf node, add our l_our_code vector to the l_codes map
		if (l_apex.type == huff_tree_node::LEAF) {
			if (m_huffman_debug) {
				std::cout << "huffman_encode: Adding symbol:" << int(l_apex.symbol) << " code:";
				std::cout << "  l_cur_code=" << l_cur_code << " l_cur_code_len=" << l_cur_code_len << "  ";
				std::uint64_t l_temp = l_cur_code;
				l_temp <<= (64 - l_cur_code_len);
				for (std::int16_t i = 0; i < l_cur_code_len; ++i) {
					bool l_bit = ((l_temp & 0x8000000000000000ULL) > 0);
					l_temp <<= 1;
					std::cout << l_bit;
				}
				std::cout << std::endl;
			}

			l_codes.insert(std::pair<uint8_t, std::pair<uint64_t, int16_t> >(l_apex.symbol, std::pair<uint64_t, int16_t>(l_cur_code, l_cur_code_len)));
			l_cur_node = l_parent.top();
			l_parent.pop();
			l_traverse_up = true;
			continue;
		}

		// we traversed down to get here and we're not a leaf node, so go down and to the left
		if (l_apex.left_id > -1) {
			l_cur_code <<=1;
			l_cur_code &= 0xfffffffffffffffeULL;
			++l_cur_code_len;
			l_parent.push(l_cur_node);
			l_cur_node = l_apex.left_id;
			if (m_huffman_debug) std::cout << "huffman_encode: traversing down to the left to id:" << l_cur_node << std::endl;
			continue;
		}
	} while (!l_finished);

	// Write out frequency table, one token for every character 0-255, of the appropriate bit width of the largest frequency count in the table:
	// 6-bit value indicating the bit width of each table item 0-64, then begin with value for char 0x00, 0x01, etc. up to 0xff..
	// pad this list by advancing to the next whole byte, then:
	// Write out m_buffer contents to another ss::data object, substiuting the bytes for the huffman codes in l_codes.

	data l_encoded;
	bit_cursor l_write_bit_cursor;
	l_encoded.set_write_bit_cursor(l_write_bit_cursor);

	// find the next greater power of 2 of l_max_freq.
	// there is an easier way of doing this in c++23 using the <bit> library
	// TODO: refactor this
	if (m_huffman_debug) std::cout << "huffman_encode: l_max_freq=" << l_max_freq << std::endl;
	std::int16_t l_maxbits = 64;
	while ((l_max_freq & 0x8000000000000000ULL) == 0) {
		l_max_freq <<= 1;
		--l_maxbits;
	}
	if (m_huffman_debug) std::cout << "huffman_encode: frequency table width=" << l_maxbits << std::endl;

	// magic cookie
	l_encoded.write_bits(HUFF_MAGIC_COOKIE, 32);

	// 64 bit data length
	l_encoded.write_bits(static_cast<std::uint64_t>(m_buffer.size()), 64);

	// frequency table symbol width
	l_encoded.write_bits(l_maxbits, 6);

	// frequency table
	for (std::uint64_t i = 0; i < 256; ++i)
		l_encoded.write_bits(l_freq[i], l_maxbits);

	// advance write cursor to next whole byte
	l_encoded.m_write_bit_cursor.advance_to_next_whole_byte();
	if (m_huffman_debug) std::cout << "huffman_encode: codes start at: (l_encoded.m_write_bit_cursor.byte)=" << l_encoded.m_write_bit_cursor.byte << std::endl;

	// write out huffman codes
	for (std::size_t i = 0; i < m_buffer.size(); ++i) {
		std::map<std::uint8_t, std::pair<std::uint64_t, std::int16_t> >::iterator l_it = l_codes.find(m_buffer[i]);
		std::uint64_t l_bits = (l_it->second).first;
		std::int16_t l_len = (l_it->second).second;
		l_encoded.write_bits(l_bits, l_len);
	}
	return l_encoded;
}

data data::huffman_decode() const
{
	data l_work = *this; // copy ourselves so we can maintain constness
	l_work.m_read_bit_cursor.set_absolute(0);

	// verify magic cookie
	std::uint32_t l_cookie = l_work.read_bits(32);
	if (l_cookie != HUFF_MAGIC_COOKIE) {
		// if our magic cookie is missing, don't even bother to decode the buffer
		data_exception e("huffman_decode: Magic cookie missing from buffer.");
		throw (e);
	}

	std::uint64_t l_datalen = l_work.read_bits(64);
	// if we read a 0 data length, just return an empty buffer
	if (l_datalen == 0) {
		data l_decoded;
		return l_decoded;
	}
	
	std::int16_t l_width = l_work.read_bits(6);

	if (m_huffman_debug) std::cout << "huffman_decode: l_datalen=" << l_datalen << " l_width=" << l_width << std::endl;

	std::uint64_t l_freq[256];
	for (std::uint64_t i = 0; i < 256; ++i)
		l_freq[i] = l_work.read_bits(l_width);

	std::deque<huff_tree_node> l_in;
	std::vector<huff_tree_node> l_out;
	std::int16_t l_out_root = -1;

	// make a node for each symbol with >0 frequency
	for (std::uint64_t i = 0; i < 256; ++i) {
		if (l_freq[i] > 0) {
			if (m_huffman_debug) std::cout << "huffman_decode: creating huff_tree_node for " << std::hex << i << std::dec << " freq=" << l_freq[i] << std::endl;
			huff_tree_node l_temp;
			l_temp.symbol = i;
			l_temp.freq = l_freq[i];
			l_in.push_back(l_temp);
		}
	}

	// arrange in ascending order of frequency
	std::sort(l_in.begin(), l_in.end());

	if (m_huffman_debug) {
		for (std::deque<huff_tree_node>::iterator l_it = l_in.begin(); l_it != l_in.end(); ++l_it)
			std::cout << "huffman_decode: sym:" << std::hex << int(l_it->symbol) << std::dec << " freq:" << l_it->freq << std::endl;
	}
	std::int16_t l_next_id = 0;

	while (l_in.size() > 1) {
		// create new internal node to point to each pulled node
		huff_tree_node l_apex;
		l_apex.type = huff_tree_node::INTERNAL;

		// get the next 2 lowest frequency symbols and assign them id's
		huff_tree_node l_left = l_in.front();
		l_in.pop_front();
		l_left.id = l_next_id++;
		l_apex.left_id = l_left.id;
		huff_tree_node l_right = l_in.front();
		l_in.pop_front();
		l_right.id = l_next_id++;
		l_apex.right_id = l_right.id;

		// assign apex sum of children's frequencies and add it back on the end of l_in
		l_apex.freq = l_left.freq + l_right.freq;
		l_in.push_back(l_apex);
		std::sort(l_in.begin(), l_in.end());

		// stick the children in our result vector
		l_out.push_back(l_left);
		l_out.push_back(l_right);

		if (m_huffman_debug) {
			std::cout << "huffman_decode: Adding: L:" << l_left.type << "-" << int(l_left.symbol) << "/" << l_left.freq;
			std::cout << " IDs: L" << l_left.left_id << " A" << l_left.id << " R" << l_left.right_id;
			std::cout << " R:" << l_right.type << "-" << int(l_right.symbol) << "/" << l_right.freq;
			std::cout << " IDs: L" << l_right.left_id << " A" << l_right.id << " R" << l_right.right_id;
			std::cout << std::endl;
		}
	}

	// there should be one node left in l_in, and this is our root node
	// Note that in the single token edge case, this singular node will be a LEAF.
	// We can leave it a LEAF instead of breaking it out with a parent root node
	// like we did in the huffman_encode method. The reason why is explained below.
	huff_tree_node l_root = l_in.front();
	l_in.pop_front();
	l_root.id = l_next_id++;
	l_out.push_back(l_root);
	l_out_root = l_root.id;

	// At this time, we should have a functional huffman tree constructed, so advance to the next whole byte
	// and start collecting bits, traversing the huffman tree until we find the leaf node that corresponds
	// to the symbol we're interested in.
	data l_decoded;
	l_decoded.m_write_bit_cursor.set_absolute(0);
	l_work.m_read_bit_cursor.advance_to_next_whole_byte();
	if (m_huffman_debug) std::cout << "huffman_decode: codes start at (m_read_bit_cursor.byte)=" << m_read_bit_cursor.byte << std::endl;

	for (std::uint64_t i = 0; i < l_datalen; ++i) {
		std::uint8_t l_symbol;
		std::int16_t l_cur = l_out_root;
		do {
			// start right at the top
			huff_tree_node l_apex = l_out[l_cur];

			// are we a leaf? if so, break out of the "do" loop
			// Note that in the single token edge case, our root node will be a LEAF anyway,
			// so this breaks out here and writes the single token below.
			if (l_apex.type == huff_tree_node::LEAF) {
				l_symbol = l_apex.symbol;
				break;
			}

			// we're on an INTERNAL node, so traverse downwards
			bool l_bit = l_work.read_bit();
			if (l_bit) {
				// 1 bit, head right
				l_cur = l_apex.right_id;
				continue;
			} else {
				// 0 bit, head left
				l_cur = l_apex.left_id;
				continue;
			}
		} while (1);
		l_decoded.write_uint8(l_symbol);
	}
	return l_decoded;
}	

data data::rle_encode() const
{
	std::uint8_t RLE_ESCAPE = 0x55;
	const std::uint8_t RLE_INCREMENT = 0x3B;

	data l_in = *this;
	data l_out;
	
	// if we're empty, nothing to do
	if (l_in.size() == 0)
		return l_out;

	std::uint8_t l_new, l_old = 0, l_count = 0;
	bool l_clear = true; // set this to indicate l_old is empty

	// sliding window RLE
	do {
		l_new = l_in.read_uint8();

		// did we encounter an escape? If so, flush the window then double it up, then rotate the escape
		if (l_new == RLE_ESCAPE) {
			if (!l_clear) {
				if (l_count > 0) {
					// if we've repeated fewer than 4 times, just write the bytes themselves
					if (l_count < 3) {
						for (std::uint64_t i = 0; i <= l_count; ++i)
							l_out.write_uint8(l_old);
						l_count = 0;
					} else {
						// write out compound set
						l_out.write_uint8(RLE_ESCAPE);
						l_out.write_uint8(l_old);
						l_out.write_uint8(l_count + 1);
						l_count = 0;
					}
				} else {
					// weren't counting when we encountered escape, so just write out l_old
					l_out.write_uint8(l_old);
				}
			}
			l_out.write_uint8(RLE_ESCAPE);
			l_out.write_uint8(RLE_ESCAPE);
			RLE_ESCAPE += RLE_INCREMENT;
			l_clear = true;
			continue;
		}

		// first time through the loop (and after escapes), just stash away the first byte
		if (l_clear) {
			l_old = l_new;
			l_clear = false;
			continue;
		}

		if (l_old == l_new) {
			++l_count;
			if (l_count == 254) { // 254 repeats = 255 characters
				// flush window and restart the count if we reached count limit
				l_out.write_uint8(RLE_ESCAPE);
				l_out.write_uint8(l_old);
				l_out.write_uint8(l_count + 1);
				l_count = 0;
				l_clear = true;
			}
		} else {
			if (l_count > 0) {
				// if we've repeated fewer than 4 times, just write the bytes themselves
				if (l_count < 3) {
					for (std::uint64_t i = 0; i <= l_count; ++i)
						l_out.write_uint8(l_old);
					l_count = 0;
				} else {
					// write out compound set
					l_out.write_uint8(RLE_ESCAPE);
					l_out.write_uint8(l_old);
					l_out.write_uint8(l_count + 1);
					l_count = 0;
				}
			} else {
				// got different byte, but no repeat, so write out old
				l_out.write_uint8(l_old);
			}
			l_old = l_new;
		}
	} while (l_in.get_read_cursor() < l_in.size());

	// flush window
	if (!l_clear) {
		if (l_count > 0) {
			if (l_count < 3) {
				for (std::uint64_t i = 0; i <= l_count; ++i)
					l_out.write_uint8(l_old);
				l_count = 0;
			} else {
				// write out compound set
				l_out.write_uint8(RLE_ESCAPE);
				l_out.write_uint8(l_old);
				l_out.write_uint8(l_count + 1);
				l_count = 0;
			}
		} else {
			l_out.write_uint8(l_old);
		}
	}
	return l_out;
}

data data::rle_decode() const
{
	std::uint8_t RLE_ESCAPE = 0x55;
	const std::uint8_t RLE_INCREMENT = 0x3B;

	data l_in = *this;
	data l_out;

	// if we're empty, nothing to do
	if (l_in.size() == 0)
		return l_out;

	std::uint64_t l_repeat = 0;
	enum { COLLECTING, FOUND_ESCAPE, FOUND_CHAR } l_state = COLLECTING;

	do {
		std::uint8_t l_new = l_in.read_uint8();
		switch (l_state) {
			case COLLECTING:
				if (l_new == RLE_ESCAPE) {
					l_state = FOUND_ESCAPE;
				} else {
					// just a normal char, so write it out
					l_out.write_uint8(l_new);
				}
				break;
			case FOUND_ESCAPE:
				if (l_new == RLE_ESCAPE) {
					// found second escape character, so write it then rotate escape
					l_out.write_uint8(l_new);
					RLE_ESCAPE += RLE_INCREMENT;
					l_state = COLLECTING;
				} else {
					// something else, must be the char we need to repeat
					l_repeat = l_new;
					l_state = FOUND_CHAR;
				}
				break;
			case FOUND_CHAR:
				if (l_new > 0) {
					l_out.fill(l_new, l_repeat);
					l_state = COLLECTING;
				} else {
					// value of 0 is illegali in a repeat construct, so data stream must be corrupted
					data_exception e("rle_decode: Illegal character in stream, possible data corruption.");
					throw (e);
				}
				break;
		}
	} while (l_in.get_read_cursor() < l_in.size());
	return l_out;
}

data data::range_encode(std::function<void(std::uint64_t, std::uint64_t)> a_status_cb)
{
	data l_work = rle_encode();
	data l_ret = l_work.range_encode_private(l_work, a_status_cb);
	return l_ret;
}

data data::range_decode(std::function<void(std::uint64_t, std::uint64_t)> a_status_cb)
{
	data l_work = range_decode_private(*this, a_status_cb);
	data l_ret = l_work.rle_decode();
	return l_ret;
}

// range coding stuff

union hidetect {
	std::uint64_t val;
	struct {
		std::uint8_t lobyte;
		std::uint8_t mid[6];
		std::uint8_t hibyte;
	};
};

struct symbol {
	std::uint32_t count;
	std::uint64_t lo;
	std::uint64_t hi;
	std::uint64_t size;
};

const std::uint16_t m_cookie = 0xaa5b;
const std::uint16_t m_cookie_multi = 0xaadc;
const std::size_t m_seg_max = 262144; // cannot exceed 16MB.. 24 bit value
std::array<symbol, 256> m_probs;
std::uint32_t m_message_len; // cannot exceed 1TB. This should be big enough for most files.
std::uint32_t m_segment_len;
std::uint64_t m_min_range;
std::uint32_t m_max_count;

void data::assign_ranges(std::uint64_t a_lo, std::uint64_t a_hi)
{
	//std::cout << "characterizing range a_lo " << std::hex << std::setfill('0') << std::setw(16) << a_lo << " a_hi " << a_hi << " size " << (a_hi - a_lo) << std::endl;
	if (a_hi == a_lo) {
//		std::cout << "can't characterize a zero size range." << std::endl;
//		exit(EXIT_FAILURE);
		data_exception e("assign_ranges: can't characterize a zero size range. Fatal error!");
		throw (e);
	}
	// assign ranges
	std::uint64_t l_lasthigh = a_lo;
	std::uint64_t l_accumulator = 0;
	m_min_range = ULLONG_MAX;
	for (std::size_t i = 0; i < 256; ++i) {
		if (m_probs[i].count > 0) {
			long double l_rangetop = (long double)(m_probs[i].count + l_accumulator) / (long double)m_segment_len;
			//std::cout << "m_probs[i].count " << std::dec << m_probs[i].count << " l_accumulator " << l_accumulator << " segment_len " << m_segment_len << " l_rangetop " << std::setprecision(15) << l_rangetop << std::endl;
			m_probs[i].lo = l_lasthigh;
			m_probs[i].hi = a_lo + (std::uint64_t)(l_rangetop * (long double)(a_hi - a_lo));
			if ((a_hi == ULLONG_MAX) && (l_rangetop == 1.0)) {
				m_probs[i].hi = ULLONG_MAX;
			}
			l_lasthigh = m_probs[i].hi + 1;
			m_probs[i].size = m_probs[i].hi - m_probs[i].lo;
			m_min_range = std::min(m_probs[i].size, m_min_range);
			//std::cout << "sym " << std::hex << (int)i << " count " << std::dec << (int)m_probs[i].count << " l_rangetop " << std::setprecision(15) << l_rangetop << std::hex << std::setfill('0') << std::setw(16) << " m_probs[i].lo " << m_probs[i].lo << " m_probs[i].hi " << m_probs[i].hi << " l_lasthigh " << l_lasthigh << std::dec << " size " << m_probs[i].size << std::endl;
			l_accumulator += m_probs[i].count;
		}
	}
	//std::cout << "m_min_range " << std::dec << m_min_range << std::endl;
}

void data::default_predicate(std::uint64_t a_num, std::uint64_t a_denom)
{
	// do nothing
	return;
}

ss::data data::range_encode_private(ss::data& a_data, std::function<void(std::uint64_t, std::uint64_t)> a_status_cb)
{
	m_message_len = a_data.size();
	std::uint16_t l_seg_count = 1;
	if (m_message_len > m_seg_max) {
		l_seg_count = (m_message_len / m_seg_max);
		if ((m_message_len % m_seg_max) > 0)
			l_seg_count++;
	}
	//std::cout << "preparing to compress " << l_seg_count << " segments." << std::endl;
	// write header
	ss::data l_comp;
	l_comp.set_network_byte_order(true);
	if (l_seg_count == 1) {
		l_comp.write_uint16(m_cookie);
	} else {
		l_comp.write_uint16(m_cookie_multi);
		l_comp.write_uint16(l_seg_count);
	}
	l_comp.write_uint40(m_message_len);

	for (std::size_t l_seg = 0; l_seg < l_seg_count; ++l_seg) {
		ss::data l_bitstream;
		l_bitstream.set_network_byte_order(true);
		std::size_t l_segstart = l_seg * m_seg_max;
		std::size_t l_segend = ((l_seg + 1) * m_seg_max);
		if (l_segend > m_message_len)
			l_segend = l_segstart + (m_message_len - (l_seg * m_seg_max));
		m_segment_len = l_segend - l_segstart;
		if (l_seg_count > 1)
			a_status_cb(l_seg, l_seg_count);
		//std::cout << "processing segment " << l_seg + 1 << " segstart " << l_segstart << " segend " << l_segend << std::endl;

		// compute probabilities
		for (std::size_t i = 0; i < 256; ++i)
			m_probs[i] = { 0, 0, 0, 0 }; // init them
//		std::cout << "characterizing probabilities for " << l_segstart << ", " << l_segend << std::endl;
		for (std::size_t i = l_segstart; i < l_segend; ++i) 
			m_probs[a_data[i]].count++;
		// find max symbol count
		m_max_count = 0;
		for (std::size_t i = 0; i < 256; ++i)
			m_max_count = std::max(m_max_count, m_probs[i].count);

		hidetect l_work_lo;
		hidetect l_work_hi;
		l_work_lo.val = 0;
		l_work_hi.val = ULLONG_MAX;
		assign_ranges(l_work_lo.val, l_work_hi.val);
		for (std::size_t i = l_segstart; i < l_segend; ++i) {
//			if ((i % 100000) == 0)
//				std::cout << ".";
			//std::cout << "encode loop: position " << i << " read symbol " << std::hex << std::setfill('0') << std::setw(2) << (int)a_data[i] << std::endl;
			l_work_lo.val = m_probs[a_data[i]].lo;
			l_work_hi.val = m_probs[a_data[i]].hi;
			assign_ranges(l_work_lo.val, l_work_hi.val);
			while (l_work_lo.hibyte == l_work_hi.hibyte) {
				//std::cout << "--- normalizing... writing " << std::hex << (int)l_work_lo.hibyte << std::endl;
				l_bitstream.write_uint8(l_work_lo.hibyte);
				l_work_lo.val <<= 8;
				l_work_hi.val <<= 8;
				l_work_hi.lobyte = 0xff;
				assign_ranges(l_work_lo.val, l_work_hi.val);
			}
		}
		l_bitstream.write_uint64(l_work_lo.val);
//		std::cout << "wrote final quadword " << std::hex << l_work_lo.val << std::endl;
//		std::cout << "compressed " << std::dec << (l_segend - l_segstart) << " symbols, compressed data len=" << l_bitstream.size() << std::endl;

		// construct full symbol count table
//		std::cout << "m_max_count " << std::dec << m_max_count << " bits " << std::bit_width(m_max_count) << std::endl;
		ss::data l_cnttbl_full;
		l_cnttbl_full.write_bit(false);
		std::uint64_t l_cntwidth = 0;
		l_cntwidth = std::bit_width(m_max_count);
		l_cnttbl_full.write_bits(l_cntwidth, 5); // 5 bit number from 0-31
		for (std::size_t i = 0; i < 256; ++i) {
			l_cnttbl_full.write_bits(m_probs[i].count, l_cntwidth);
		}

		// construct enumerated count table
		ss::data l_cnttbl_enum;
		l_cnttbl_enum.write_bit(true);
		l_cnttbl_enum.write_bits(l_cntwidth, 5);
		std::uint8_t l_enum_entries = 0;
		for (std::size_t i = 0; i < 256; ++i)
			if (m_probs[i].count > 0)
				l_enum_entries++;
		l_cnttbl_enum.write_bits(l_enum_entries, 8);
		for (std::size_t i = 0; i < 256; ++i) {
			if (m_probs[i].count > 0) {
				l_cnttbl_enum.write_bits(i, 8);
				l_cnttbl_enum.write_bits(m_probs[i].count, l_cntwidth);
			}
		}

		l_comp.write_uint24(l_bitstream.size());
		// append frequency table, whichever one is smaller
//		std::cout << "cnttbl_full " << std::dec << l_cnttbl_full.size() << " cnttbl_enum " << l_cnttbl_enum.size() << std::endl;
		if (l_cnttbl_full.size() < l_cnttbl_enum.size()) {
			l_comp += l_cnttbl_full;
		} else {
			l_comp += l_cnttbl_enum;
		}
		// append bitstream
		l_comp += l_bitstream;
	}

//	std::cout << "full compressed package (with header+count table) " << std::dec << l_comp.size() << " bytes." << std::endl;
	return l_comp;
}

ss::data data::range_decode_private(ss::data& a_data, std::function<void(std::uint64_t, std::uint64_t)> a_status_cb)
{
	ss::data l_ret;

	a_data.set_network_byte_order(true); // just to be on the safe side
	std::uint16_t l_cookie = a_data.read_uint16();
	if ((l_cookie != m_cookie) && (l_cookie != m_cookie_multi)) {
		// cookie error
//		std::cout << "Cookie mismatch" << std::endl;
//		exit(EXIT_FAILURE);
		data_exception e("range_decode: cookie mismatch");
		throw (e);
	}
	std::uint16_t l_seg_count = 1;
	if (l_cookie == m_cookie_multi) {
		l_seg_count = a_data.read_uint16();
	}
	std::uint64_t l_original_size = a_data.read_uint40();
//	std::cout << "original size " << std::dec << l_original_size << std::endl;

	for (std::size_t l_seg = 0; l_seg < l_seg_count; ++l_seg) {
		std::size_t l_segstart = l_seg * m_seg_max;
		std::size_t l_segend = ((l_seg + 1) * m_seg_max);
		if (l_segend > l_original_size)
			l_segend = l_segstart + (l_original_size - (l_seg * m_seg_max));
		m_segment_len = l_segend - l_segstart;
		std::uint32_t l_seg_bitstream_size = a_data.read_uint24();
		if (l_seg_count > 1)
			a_status_cb(l_seg, l_seg_count);
		//std::cout << "decoding segment " << l_seg + 1 << " segstart " << l_segstart << " segend " << l_segend << " size " << m_segment_len << " bitstream size " << l_seg_bitstream_size << std::endl;
	
		// read count table
		ss::data::bit_cursor l_bitcursor;
		l_bitcursor.byte = a_data.get_read_cursor(); // step over header data
		a_data.set_read_bit_cursor(l_bitcursor);
		bool l_is_enumerated = a_data.read_bit();
		if (l_is_enumerated) {
//			std::cout << "reading enumerated count table..." << std::endl;
			// read enumerated count table
			std::uint64_t l_cntwidth = a_data.read_bits(5);
			std::uint8_t l_enum_entries = a_data.read_bits(8);
			// clear the table first
			for (std::size_t i = 0; i < 256; ++i) {
				m_probs[i].count = 0;
				m_probs[i].lo = 0;
				m_probs[i].hi = 0;
				m_probs[i].size = 0;
			}
			for (std::size_t i = 0; i < l_enum_entries; ++i) {
				std::uint8_t l_symbol = a_data.read_bits(8);
				std::uint32_t l_frequency = a_data.read_bits(l_cntwidth);
				m_probs[l_symbol].count = l_frequency;
			}
		} else {
//			std::cout << "reading full count table..." << std::endl;
			// read full count table
			std::uint64_t l_cntwidth = a_data.read_bits(5);
			for (std::size_t i = 0; i < 256; ++i) {
				m_probs[i].count = a_data.read_bits(l_cntwidth);
				m_probs[i].lo = 0;
				m_probs[i].hi = 0;
				m_probs[i].size = 0;
			}
		}
		assign_ranges(0, ULLONG_MAX);
		l_bitcursor = a_data.get_read_bit_cursor();
		l_bitcursor.advance_to_next_whole_byte();
		a_data.set_read_bit_cursor(l_bitcursor);
		a_data.set_read_cursor(l_bitcursor.byte);

		// decode
		std::uint32_t l_bitstream_pos = 0;
		hidetect l_work;
       		l_work.val = a_data.read_uint64(); // prime the pump
		l_bitstream_pos += 8;
		hidetect l_lo;
		hidetect l_hi;
		l_lo.val = 0;
		l_hi.val = ULLONG_MAX;
//		std::cout << "range_decode: starting with work value " << std::hex << std::setfill('0') << std::setw(16) << l_work.val << std::endl;

		std::uint32_t l_pos = 0;
		while (1) {
			bool l_found = false;
			for (std::size_t i = 0; i < 256; ++i) {
				if ((l_work.val <= m_probs[i].hi) && (l_work.val >= m_probs[i].lo) && (m_probs[i].count > 0)) {
					//std::cout << "pos " << std::dec << l_pos << " decoded symbol " << std::hex << std::setw(2) << i << " = " << (char)i << std::endl;
					l_ret.write_uint8(i);
					l_pos++;
					l_found = true;
					// if the high byte of the ranges match, scoot the values over and read in next byte from the stream to be the lobyte
					l_lo.val = m_probs[i].lo;
					l_hi.val = m_probs[i].hi;
					assign_ranges(l_lo.val, l_hi.val);
					while (l_lo.hibyte == l_hi.hibyte) {
						l_lo.val <<= 8;
						l_hi.val <<= 8;
						l_hi.lobyte = 0xff;
						l_work.val <<= 8;
						l_work.lobyte = a_data.read_uint8();
						l_bitstream_pos++;
						//std::cout << "---read from bitstream: " << std::hex << (int)l_work.lobyte << " bitstream_pos " << std::dec << l_bitstream_pos << std::endl;
					}
					assign_ranges(l_lo.val, l_hi.val);
					break;
				}
			}
			if (l_pos >= m_segment_len)
				break;
			if (!l_found) {
				// exhausted the for loop... didn't find the range. Fatal error
				std::cout << "unable to find range for work " << std::hex << l_work.val << " at pos " << std::dec << l_pos << std::endl;
				std::cout << "l_lo.val " << std::hex << l_lo.val << " l_hi.val " << l_hi.val << std::endl;
				for (std::size_t i = 0; i < 256; ++i) {
					if (m_probs[i].count > 0) {
						std::cout << "sym " << std::hex << (int)i << " count " << std::dec << std::setfill(' ') << std::setw(0) << (int)m_probs[i].count << std::hex << std::setfill('0') << std::setw(16) << " l_sym.lo " << m_probs[i].lo << " l_sym.hi " << m_probs[i].hi << std::dec << " size " << m_probs[i].size << std::endl;
					}
				}
				exit(EXIT_FAILURE);
			}
		}
		while (l_seg_bitstream_size > l_bitstream_pos) {
			a_data.read_uint8(); // throw away
//			std::cout << "throwing away " << std::hex << l_throwaway << " from bitstream" << std::endl;
		}
	}

	return l_ret;
}

};

