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
#include <random>
#include <optional>

#include <cstdint>

#include "bf.h"
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
	
	typedef union {
		long double longdouble_val;
		std::uint8_t raw[16];
	} size16_union;

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
	std::size_t size() const { return m_buffer.size(); };

	void fill(std::size_t a_num_bytes, std::uint8_t a_val);
	void random(std::size_t a_num_bytes);
	void clear();
	void truncate_back(std::size_t a_new_len);
	void assign(std::uint8_t *a_buffer, std::size_t a_len);
	bool compare(const data& a_data) const; // true = same, false = different
	
	/* operators */
	bool operator==(const data& rhs) const; // convenience wrappers for compare() function
	bool operator!=(const data& rhs) const;
	
	/* basic C/C++ inbuilt types */
	
	void write_uint8(std::uint8_t a_uint8);
	std::uint8_t read_uint8();
	void write_int8(std::int8_t a_int8);
	std::int8_t read_int8();

	void write_uint16(std::uint16_t a_uint16);
	std::uint16_t read_uint16();
	void write_int16(std::int16_t a_int16);
	std::int16_t read_int16();
	
	void write_uint32(std::uint32_t a_uint32);
	std::uint32_t read_uint32();
	void write_int32(std::int32_t a_int32);
	std::int32_t read_int32();

	void write_uint64(std::uint64_t a_uint64);
	std::uint64_t read_uint64();
	void write_int64(std::int64_t a_int64);
	std::int64_t read_int64();

	void write_float(float a_float);
	float read_float();
	void write_double(double a_double);
	double read_double();
	// note: platform-specific 16 byte double, ignores endianness setting
	void write_longdouble(long double a_longdouble);
	long double read_longdouble();
	
	/* strings */
	
	void set_delimiter(std::uint8_t a_delimiter) { m_delimiter = a_delimiter; };
	std::uint8_t get_delimiter() { return m_delimiter; };
	
	void write_std_str(const std::string& a_str);
	std::string read_std_str(std::size_t a_length);
	void write_std_str_delim(const std::string& a_str);
	std::optional<std::string> read_std_str_delim();
	
	/* hashing */
	
	std::uint32_t crc32(std::uint32_t a_crc);
	data md5();
	data sha1();
	data sha2_224();
	data sha2_256();
	data sha2_384();
	data sha2_512();
	
	/* encryption */
	
	static data bf_key_random();
	static data bf_key_schedule(const std::string& a_string);
	static data bf_block_encrypt(data& a_block, data& a_key);
	static data bf_block_decrypt(data& a_block, data& a_key);

protected:
	bool m_network_byte_order;
	std::size_t m_read_cursor;
	std::size_t m_write_cursor;
	std::vector<std::uint8_t> m_buffer;
	std::uint8_t m_delimiter;
};

}; // namespace ss

#endif // SS2XDATA_H

