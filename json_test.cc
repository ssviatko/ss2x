#include <iostream>
#include <string>
#include <format>
#include <memory>
#include <filesystem>
#include <stdexcept>

#include "log.h"
#include "data.h"
#include "fs.h"
#include "json.h"

void display_array(ss::json::array_ptr a_array);

void display_object(ss::json::object_ptr a_object)
{
	std::cout << "--- object size: " << a_object->stringvalues.size() << std::endl;
	for (const auto& a : a_object->stringvalues) {
		std::cout << "name: " << a.first->content();
		std::cout << "  value type: ";
		switch (a.second->type()) {
			case ss::json::element_type::STRING:
				std::cout << "STRING";
				std::cout << "  value: " << a.second->content() << std::endl;
				break;
			case ss::json::element_type::NUMBER:
				std::cout << "NUMBER";
				std::cout << "  floatvalue: " << ss::json::as_number(a.second)->as_float();
				std::cout << "  intvalue: " << ss::json::as_number(a.second)->as_int();
				std::cout << "  uintvalue: " << ss::json::as_number(a.second)->as_uint() << std::endl;
				break;
			case ss::json::element_type::OBJECT:
				std::cout << "OBJECT" << std::endl;
				display_object(ss::json::as_object(a.second));
				break;
			case ss::json::element_type::ARRAY:
				std::cout << "ARRAY" << std::endl;
				display_array(ss::json::as_array(a.second));
				break;
			case ss::json::element_type::TRUE:
			case ss::json::element_type::FALSE:
			case ss::json::element_type::NULLELEMENT:
				std::cout << "CONSTANT";
				std::cout << "  element type: " << a.second->content() << std::endl;
				break;
		}
	}
	std::cout << "--- end of object" << std::endl;
}

void display_array(ss::json::array_ptr a_array)
{
	std::cout << "--- array size: " << a_array->values.size() << std::endl;
	for (const auto& a : a_array->values) {
		std::cout << "value type: ";
		switch (a->type()) {
			case ss::json::element_type::STRING:
				std::cout << "STRING";
				std::cout << "  value: " << a->content() << std::endl;
				break;
			case ss::json::element_type::NUMBER:
				std::cout << "NUMBER";
				std::cout << "  floatvalue: " << ss::json::as_number(a)->as_float();
				std::cout << "  intvalue: " << ss::json::as_number(a)->as_int();
				std::cout << "  uintvalue: " << ss::json::as_number(a)->as_uint() << std::endl;
				break;
			case ss::json::element_type::OBJECT:
				std::cout << "OBJECT" << std::endl;
				display_object(ss::json::as_object(a));
				break;
			case ss::json::element_type::ARRAY:
				std::cout << "ARRAY" << std::endl;
				display_array(ss::json::as_array(a));
				break;
			case ss::json::element_type::TRUE:
			case ss::json::element_type::FALSE:
			case ss::json::element_type::NULLELEMENT:
				std::cout << "CONSTANT";
				std::cout << "  element type: " << a->content() << std::endl;
				break;
		}
	}
	std::cout << "--- end of array" << std::endl;
}

void display_master(ss::json::master_ptr a_master)
{
	std::cout << "master type: ";
	if (a_master->type() == ss::json::element_type::OBJECT) {
		std::cout << "OBJECT" << std::endl;
		display_object(a_master->as_object());
	}
	if (a_master->type() == ss::json::element_type::ARRAY) {
		std::cout << "ARRAY" << std::endl;
		display_array(a_master->as_array());
	}
}

/* JSON serialization examples */

class json_vector : public std::vector<int>, public ss::json::json_serializable {
public:
	json_vector(const std::string& a_name) : ss::json::json_serializable(a_name, ss::json::element_type::ARRAY) { }
	~json_vector() { }
	std::string seal();
};

std::string json_vector::seal()
{
	std::string l_ret;
	std::vector<int>::iterator this_it = this->begin();
	while (this_it != this->end()) {
		if (this_it == (this->end() - 1)) {
			l_ret += value_number(*this_it);
		} else {
			l_ret += value_number(*this_it) + ", ";
		}
		++this_it;
	}
	return l_ret;
}

class json_a : public ss::json::json_serializable {
public:
	json_a(const std::string& a_name) : ss::json::json_serializable(a_name, ss::json::element_type::OBJECT) { vec_a.push_back(111); vec_a.push_back(325); }
	int int_a;
	int int_b;
	std::string string_a;
	std::string string_b;
	json_vector vec_a = json_vector("vec_a");
	std::string seal();
};

std::string json_a::seal()
{
	return ss::json::comma_list(stringvalue_number("int_a", int_a),
		stringvalue_number("int_b", int_b),
		stringvalue_string("string a", string_a),
		stringvalue_string("string b", string_b),
		stringvalue_array(vec_a));
}

class json_b : public ss::json::json_serializable {
public:
	json_b(const std::string& a_name, json_a& a_jsona_a, json_a& a_jsona_b) : ss::json::json_serializable(a_name, ss::json::element_type::OBJECT),
		jsona_a(a_jsona_a), jsona_b(a_jsona_b) { }
	int int_a;
	double f_a;
	double f_b;
	json_a& jsona_a;
	json_a& jsona_b;
	std::string seal();
};

std::string json_b::seal()
{
	return ss::json::comma_list(stringvalue_number("int_a", int_a),
		stringvalue_number("f, a", f_a),
		stringvalue_number("f_b", f_b),
		jsona_a.serialize(),
		jsona_b.serialize());
}

int main(int argc, char **argv)
{
	ss::failure_services& fs = ss::failure_services::get();
	fs.install_signal_handler();
	
	ss::log::ctx& ctx = ss::log::ctx::get();
	ctx.register_thread("main");
	std::shared_ptr<ss::log::target_stdout> l_stdout =
		std::make_shared<ss::log::target_stdout>(ss::log::DEBUG, ss::log::target_stdout::DEFAULT_FORMATTER);
	ctx.add_target(l_stdout, "default");

	for (const auto& l_file : std::filesystem::recursive_directory_iterator("./json/")) {
		if ((l_file.is_regular_file()) && (!(l_file.is_symlink()))) {
			ctx.log(std::format("loading json file: {}", l_file.path().string()));
			ss::data l_json_file;
			l_json_file.load_file(l_file.path());
			std::string l_json = l_json_file.read_std_str(l_json_file.size());
			try {
				l_json = ss::json::make_human_readable(l_json);
				ctx.log(std::format("processing json: {}", l_json));
				ss::json::master_ptr l_master = ss::json::parse_json(l_json);
				display_master(l_master);
			} catch (std::invalid_argument& e) {
				ctx.log(std::format("Caught exception: {}", e.what()));
			}
		}
	}
	
	// serialize json object graph
	json_a ja1("ja_1 \\\" \\yes");
	ja1.int_a = 7;
	ja1.int_b = 3;
	ja1.string_a = "foo";
	ja1.string_b = "fubar";
	json_a ja2("ja_2");
	ja2.int_a = 99;
	ja2.int_b = 44;
	ja2.string_a = "bohica";
	ja2.string_b = "pebkac";
	json_b jb("jb_main", ja1, ja2);
	jb.int_a = 27;
	json_b jb2("jb_alt", ja1, ja2);
	jb2.int_a = 434;
	
	std::string l_jb_json = ss::json::json_serializable::master_object_enclose(jb.serialize() + ", " + jb2.serialize());
	ctx.log(std::format("jb serialized to json is: {}", ss::json::make_human_readable(l_jb_json)));
	ss::json::master_ptr l_jbmaster = ss::json::parse_json(l_jb_json);
	display_master(l_jbmaster);
	
	json_vector jv("jv1");
	jv.push_back(6);
	jv.push_back(9);
	jv.push_back(20);
	std::string l_jv_json = jv.serialize();
	ctx.log(std::format("jv serialized to json is: {}", ss::json::make_human_readable(l_jv_json)));
	ss::json::master_ptr l_jvmaster = ss::json::parse_json(l_jv_json);
	display_master(l_jvmaster);
	
	
	return 0;
}

