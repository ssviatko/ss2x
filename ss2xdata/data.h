#ifndef SS2XDATA_H
#define SS2XDATA_H

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <exception>
#include <bit>
#include <algorithm>

#include <cstdint>

#include "md5.h"
#include "sha1.h"
#include "sha2.h"

namespace ss {

class data_exception : public std::exception {
public:
	data_exception(const char* a_what) throw();
	data_exception(const std::string& a_what) throw();
	~data_exception() throw() { } // Destroy exception object.
	virtual const char* what() const throw();
	
protected:
	std::string m_what; // Error string.
};

class data {

	const static std::uint32_t crc32_tab[];

	typedef union {
		std::uint8_t uint8_val;
		std::int8_t int8_val;
	} size1_union;

	typedef union {
		std::uint16_t uint16_val;
		std::int16_t int16_val;
		std::uint8_t raw[2];
	} size2_union;

	typedef union {
		std::uint32_t uint32_val;
		std::int32_t int32_val;
		float float_val;
		std::uint8_t raw[4];
	} size4_union;

	typedef union {
		std::uint64_t uint64_val;
		std::int64_t int64_val;
		double double_val;
		std::uint8_t raw[8];
	} size8_union;

	std::vector<std::uint8_t> read_raw_data(std::size_t a_num_bytes);
	void write_raw_data(std::vector<std::uint8_t>& a_vector);
	
public:
	data();
	~data();
	
	void dump_hex();
	std::string as_hex_str();
	std::string as_hex_str_nospace();
	
	void save_file(const std::string& a_filename);
	void load_file(const std::string& a_filename);

	void set_write_cursor(std::size_t a_write_cursor);
	void set_write_cursor_to_append();
	void set_read_cursor(std::size_t a_read_cursor);
	void set_network_byte_order(bool a_setting) { m_network_byte_order = a_setting; };
	
	std::size_t get_write_cursor() { return m_write_cursor; };
	std::size_t get_read_cursor() { return m_read_cursor; };
	bool get_network_byte_order() { return m_network_byte_order; };
	std::size_t size() { return m_buffer.size(); };

	void fill(std::size_t a_num_bytes, std::uint8_t a_val);
	
	void write_uint8(std::uint8_t a_uint8);
	std::uint8_t read_uint8();
	void write_int8(std::int8_t a_int8);
	std::int8_t read_int8();
	void write_uint32(std::uint32_t a_uint32);
	std::uint32_t read_uint32();

	std::uint32_t crc32(std::uint32_t a_crc);
	data md5();
	data sha1();
	data sha2_224();
	data sha2_256();
	data sha2_384();
	data sha2_512();

protected:
	bool m_network_byte_order;
	std::size_t m_read_cursor;
	std::size_t m_write_cursor;
	std::vector<std::uint8_t> m_buffer;
};

}; // namespace ss

#endif // SS2XDATA_H

