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
				ctx.log(std::format("processing json: {}", l_json));
				ss::json::master_ptr l_master = ss::json::parse_json(l_json);
				display_master(l_master);
			} catch (std::invalid_argument& e) {
				ctx.log(std::format("Caught exception: {}", e.what()));
			}
		}
	}
	return 0;
}

