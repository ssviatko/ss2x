#include "icr.h"

namespace ss {

/* database manager singleton */

icr& icr::get()
{
	static icr shared_instance;
	return shared_instance;
}

icr::icr()
{
}

icr::~icr()
{
	shutdown();
}

void icr::shutdown()
{
	// iterate list of files and process each one, with do_update set to true
	std::vector<std::string>::iterator m_files_it = m_files.begin();
	while (m_files_it != m_files.end()) {
		process_file((*m_files_it), true);
		++m_files_it;
	}	
}

void icr::restart()
{
	shutdown();
	m_categories.clear();
	m_files.clear();
	m_argv.clear();
	m_exename = "";
}

void icr::set_keyvalue(const std::string& a_category, const std::string& a_key, const std::string& a_value)
{
	std::shared_ptr<std::map<std::string, std::string> > l_cat;
	// find category's map
	std::map<std::string, std::shared_ptr<std::map<std::string, std::string> > >::iterator m_categories_it = m_categories.find(a_category);

	// if category doesn't exist, create one
	if (m_categories_it == m_categories.end()) {
		l_cat = std::make_shared<std::map<std::string, std::string> >();
		m_categories.insert(std::pair<std::string, std::shared_ptr<std::map<std::string, std::string> > >(a_category, l_cat));
	} else {
		l_cat = m_categories_it->second;
	}

	// insert key/value pair into category, overwriting key/value pair if it exists
	std::map<std::string, std::string>::iterator l_cat_it = l_cat->find(a_key);
	if (l_cat_it != l_cat->end())
		l_cat->erase(l_cat_it);
	l_cat->insert(std::pair<std::string, std::string>(a_key, a_value));
}

std::string icr::keyvalue(const std::string& a_category, const std::string& a_key)
{
	std::shared_ptr<std::map<std::string, std::string> > l_cat;
	// find category's map
	std::map<std::string, std::shared_ptr<std::map<std::string, std::string> > >::iterator m_categories_it = m_categories.find(a_category);

	// if category doesn't exist, return an empty string
	if (m_categories_it == m_categories.end())
		return "";
	else
		l_cat = m_categories_it->second;

	// search category for key
	std::map<std::string, std::string>::iterator l_cat_it = l_cat->find(a_key);

	// if key not found, return empty string
	if (l_cat_it == l_cat->end())
		return "";
	else
		return l_cat_it->second;
}

std::vector<std::string> icr::categories()
{
	std::vector<std::string> l_ret;

	// assemble return vector
	std::map<std::string, std::shared_ptr<std::map<std::string, std::string> > >::iterator m_categories_it = m_categories.begin();
	while (m_categories_it != m_categories.end()) {
		l_ret.push_back(m_categories_it->first);
		++m_categories_it;
	}
	return l_ret;
}

bool icr::category_is_defined(const std::string& a_category)
{
	std::map<std::string, std::shared_ptr<std::map<std::string, std::string> > >::iterator m_categories_it = m_categories.find(a_category);
	return (m_categories_it != m_categories.end());
}

std::vector<std::string> icr::keys_for_category(const std::string& a_category)
{
	std::vector<std::string> l_ret;

	// find category's map
	std::map<std::string, std::shared_ptr<std::map<std::string, std::string> > >::iterator m_categories_it = m_categories.find(a_category);

	// if category doesn't exist, return the empty vector
	if (m_categories_it == m_categories.end())
		return l_ret;

	// assemble return vector
	std::map<std::string, std::string>::iterator kvlist_it = m_categories_it->second->begin();
	while (kvlist_it != m_categories_it->second->end()) {
		l_ret.push_back(kvlist_it->first);
		++kvlist_it;
	}
	return l_ret;
}

bool icr::key_is_defined(const std::string& a_category, const std::string& a_key)
{
	std::shared_ptr<std::map<std::string, std::string> > l_cat;

	// find category's map
	std::map<std::string, std::shared_ptr<std::map<std::string, std::string> > >::iterator m_categories_it = m_categories.find(a_category);

	// if category doesn't exist, return false
	if (m_categories_it == m_categories.end())
		return false;
	else
		l_cat = m_categories_it->second;

	// search category for key
	std::map<std::string, std::string>::iterator l_cat_it = l_cat->find(a_key);

	// return result of find operation
	return (l_cat_it != l_cat->end());
}

void icr::read_file(const std::string& a_filename, bool update_on_close)
{
	// if we want to update on close, add this file to the list
	if (update_on_close)
		m_files.push_back(a_filename);

	// read in all categories/keys/values contained in a_filename
	process_file(a_filename, false);
}

void icr::process_file(const std::string& a_filename, bool do_update)
{
	// read the file into memory
	std::vector<std::string> l_lines;
	const int rd_buff_size = 4096;
	std::array<std::uint8_t, rd_buff_size> l_buff;
	std::ifstream infile;

	infile.open(a_filename.c_str(), std::ifstream::in);
	if (infile.is_open()) {
		// load up entire file line by line
		while (true) {
			infile.getline((char *)l_buff.data(), rd_buff_size - 1, '\n');
			if (infile.eof())
				break;
			std::string l_line((char *)l_buff.data());
			l_lines.push_back(l_line);
		}
		infile.close();
	}

	// iterate the file, looking for key/value pairs
	std::string l_current_category;
	std::vector<std::string>::iterator l_lines_it = l_lines.begin();
	while (l_lines_it != l_lines.end()) {
		std::string l_line = (*l_lines_it);
		// attempt to categorize this line
		if (l_line.size() == 0) {
			// empty line
		} else if (l_line[0] == '#') {
			// comment
		} else if (l_line[0] == '+') {
			// import - process file specified after the plus sign
			std::string l_import = l_line.substr(1);
			process_file(l_import, false); // all imported files are read-only
		} else if (l_line[0] == '[') {
			// heading specifier - collect all text up to closing bracket
			size_t pos = l_line.find("]");
			if (pos != std::string::npos) {
				// collect characters from position 1 to pos-1
				l_current_category = l_line.substr(1, pos - 1);
				env_expand(l_current_category); // and expand it
			}
		} else {
			// line is not empty, but does not start with a hash or open bracket.. so attempt to parse it as a key/value pair
			size_t l_equals = l_line.find("="); // find equals sign
			if (l_equals != std::string::npos) {
				// that's a good sign if we found an equals sign. Assemble key from left side, value from right side, and trim spaces.
				std::string l_key = l_line.substr(0, l_equals);
				std::string l_value = l_line.substr(l_equals + 1);
				trim_std_string(l_key);
				trim_std_string(l_value);
				if ((l_key.size() > 0) && (l_value.size() > 0)) {
					// okay, we have two strings...
					bool l_env_expanded = env_expand(l_key) | env_expand(l_value); // expand any environment tokens
					if ((do_update) && (!l_env_expanded)) {
						// find value that corresponds with category/key from db, then update line.
						// DON'T do this if either the key or the value have been expanded from an env variable.
						std::string l_reg_value = keyvalue(l_current_category, l_key);
						if (l_reg_value != "") {
							// if cat/key actually resolves to something, update the line
							(*l_lines_it) = l_key + " = " + l_reg_value;
						}
					} else {
						// insert category/key/value into registry db
						set_keyvalue(l_current_category, l_key, l_value);
					}
				}
			}
		}
		++l_lines_it;
	}

	// write the file back out, if we are updating
	if (do_update) {
		std::ofstream outfile;
		outfile.open(a_filename.c_str(), std::ofstream::out | std::ofstream::trunc);
		if (outfile.is_open()) {
			std::vector<std::string>::iterator l_lines_it = l_lines.begin();
			while (l_lines_it != l_lines.end()) {
				outfile << (*l_lines_it) << std::endl;
				++l_lines_it;
			}
			outfile.close();
		}
	}
}

std::vector<std::string> icr::files()
{
	// return a copy of our files list
	std::vector<std::string> l_ret = m_files;
	return l_ret;
}

void icr::read_arguments(int a_argc, char **a_argv)
{
	// populate m_exename
	m_exename = std::string(a_argv[0]);
	
	// if we have arguments, process them for --set_keyvalue= switches
	if (a_argc > 1) {
		for (int i = 1; i < a_argc; ++i) {
			std::string l_arg = a_argv[i];
			m_argv.push_back(l_arg);
			if (l_arg.substr(0, 15) == "--set_keyvalue=") {
				l_arg = l_arg.substr(15); // trim off command
				// enforce structure of 3 parts of string, delimited by 2 commas. If either or both commas are missing, bail out
				size_t l_comma1 = l_arg.find(",");
				size_t l_comma2 = l_arg.find(",", l_comma1 + 1);
				if ((l_comma1 != std::string::npos) && (l_comma2 != std::string::npos)) {
					std::string l_category = l_arg.substr(0, l_comma1);
					std::string l_key = l_arg.substr(l_comma1 + 1, l_comma2 - l_comma1 - 1);
					std::string l_value = l_arg.substr(l_comma2 + 1);
					trim_std_string(l_category);
					trim_std_string(l_key);
					trim_std_string(l_value);
					set_keyvalue(l_category, l_key, l_value);
				}
			}
		}
	}
}

/* adaptors: registry extraction */

bool icr::to_boolean(const std::string& a_str)
{
	std::string l_str = a_str;

	// trim string, then convert to lowercase
	trim_std_string(l_str);
	std::transform(l_str.begin(), l_str.end(), l_str.begin(), tolower);

	if ((l_str == "true") || (l_str == "yes"))
		return true;
	else
		return false; // anything other than the word "true" or "yes" resolves to false.
}

int64_t icr::to_integer(const std::string& a_str)
{
	std::stringstream ss(a_str);
	int64_t l_ret;
	ss >> l_ret;
	return l_ret;
}

double icr::to_float(const std::string& a_str)
{
	std::stringstream ss(a_str);
	double l_ret;
	ss >> l_ret;
	return l_ret;
}

std::string icr::to_unformatted_string(const std::string& a_str)
{
	std::string l_ret;
	enum { COLLECT, ESCAPE } l_state;

	l_state = COLLECT;
	for (unsigned int i = 0; i < a_str.size(); ++i) {
		if (l_state == COLLECT) {
			char l_next = a_str[i];
			if (l_next == '\\')
				l_state = ESCAPE;
			else
				l_ret += l_next;
		}
		else if (l_state == ESCAPE) {
			char l_next = a_str[i];
			switch (l_next) {
				case '\"':
					l_ret += '\"';
					break;
				case 'n':
					l_ret += '\n';
					break;
			}
			l_state = COLLECT;
		}
	}
	return l_ret;
}

std::vector<std::string> icr::to_string_list(const std::string& a_str)
{
	std::vector<std::string> l_ret;
	enum { COLLECT, QUOTED, SPLIT } l_state;
	std::string l_current;
	std::string l_str = a_str + ","; // tack a comma on the end of our work string to make it easier on our state machine

	l_state = COLLECT;
	for (unsigned int i = 0; i < l_str.size(); ++i) {
		char l_next = l_str[i];
		if (l_state == COLLECT) {
			if (l_next == '\"') {
				// discard quote character and switch to quote mode
				l_state = QUOTED;
			} else if (l_next == ',') {
				// split character, trim current string and insert it into vector
				trim_std_string(l_current);
				l_ret.push_back(l_current);
				l_current = "";
				l_state = COLLECT;
			} else {
				// append this character
				l_current += l_next;
			}
		} else if (l_state == QUOTED) {
			if (l_next == '\"') {
				// discard quote character and switch back to collect mode
				l_state = COLLECT;
			} else {
				// append this character, even if it is a comma
				l_current += l_next;
			}
		}
	}
	return l_ret;
}

/* adaptors: registry insertion */

std::string icr::from_boolean(bool a_bool)
{
	std::string l_ret;

	if (a_bool)
		l_ret = "true";
	else
		l_ret = "false";

	return l_ret;
}

std::string icr::from_integer(int64_t a_int)
{
	std::stringstream ss;
	ss << a_int;
	return ss.str();
}

std::string icr::from_float(double a_double)
{
	std::stringstream ss;
	ss << a_double;
	return ss.str();
}

std::string icr::from_unformatted_string(const std::string& a_str)
{
	std::string l_ret;

	for (unsigned int i = 0; i < a_str.size(); ++i) {
		char l_next = a_str[i];
		switch (l_next) {
			case '\"':
				l_ret += "\\\"";
				break;
			case '\n':
				l_ret += "\\n";
				break;
			default:
				l_ret += l_next;
				break;
		}
	}
	return l_ret;
}

std::string icr::from_string_list(const std::vector<std::string>& a_vec)
{
	std::string l_ret;

	if (a_vec.size() > 0) {
		std::vector<std::string>::const_iterator a_vec_it = a_vec.begin();
		do {
			l_ret += (*a_vec_it);
			++a_vec_it;
			if (a_vec_it != a_vec.end())
				l_ret += ", ";
		} while (a_vec_it != a_vec.end());
	}
	return l_ret;
}

/* utility functions */

void icr::trim_std_string(std::string& a_str)
{
	// trim front
	size_t l_first = a_str.find_first_not_of(" \t\n");
	if (l_first == std::string::npos) {
		a_str.clear(); // string is all whitespace
		return;
	} else if (l_first != 0) {
		a_str.erase(0, l_first); // whitespace exists at beginning
	}

	// trim rear
	size_t l_last = a_str.find_last_not_of(" \t\n");
	if ((l_last != std::string::npos) && (l_last < a_str.size()))
		a_str.erase(l_last + 1);
}

bool icr::env_expand(std::string& a_str)
{
	// expand environment variable tokens to their full text. Return true if we did an expansion.
	bool l_ret = false;
	size_t found_opener = 0;

	while (1) {
		found_opener = a_str.find("${", found_opener);
		if (found_opener == std::string::npos)
			break;
		// find our closer
		size_t found_closer = a_str.find("}", found_opener + 2);
		if (found_closer == std::string::npos) {
			// an opener without a closer.. error!
			break;
		}
		std::string l_env = a_str.substr(found_opener + 2, found_closer - found_opener - 2);
		char *l_exp_p = getenv(l_env.c_str());
		std::string l_exp;
		if (l_exp_p != NULL)
			l_exp = l_exp_p;
		a_str.replace(found_opener, (found_closer - found_opener + 1), l_exp);
		// increment our found_opener to step over the breadth of the string we just substituted
		found_opener += l_exp.size();
		l_ret = true;
	}

	return l_ret;
}

} // namespace ss

