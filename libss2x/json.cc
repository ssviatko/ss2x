#include "json.h"

namespace ss {
namespace json {

	/* not needed in the non-library version
	 * 
std::string buildno()
{
	return BUILD_NUMBER;
}

std::string builddate()
{
	return BUILD_DATE;
}
*/

/* element */

element::element()
{
	
}

element::element(const std::string& a_content, element_type a_type)
: m_content(a_content)
, m_type(a_type)
{
	
}

element::~element()
{
	
}

/* constant */

constant::constant(element_type a_type)
: element("", a_type)
{
	switch (a_type) {
		case element_type::TRUE:
			m_content = "true";
			break;
		case element_type::FALSE:
			m_content = "false";
			break;
		case element_type::NULLELEMENT:
			m_content = "null";
			break;
		// to shut up compiler
		case element_type::ARRAY:
		case element_type::STRING:
		case element_type::NUMBER:
		case element_type::OBJECT:
			break;
	}
}

constant::~constant()
{
	
}

/* number */

number::number()
: element("", element_type::NUMBER)
{
	
}

number::number(const std::string& a_content)
: element(a_content, element_type::NUMBER)
{
	
}

number::~number()
{
	
}

std::uint64_t number::as_uint()
{
	std::stringstream ss(m_content);
	std::uint64_t l_ret;
	ss >> l_ret;
	return l_ret;
}

std::int64_t number::as_int()
{
	std::stringstream ss(m_content);
	std::int64_t l_ret;
	ss >> l_ret;
	return l_ret;
}

double number::as_float()
{
	std::stringstream ss(m_content);
	double l_ret;
	ss >> l_ret;
	return l_ret;
}

/* string */

string::string()
: element("", element_type::STRING)
{
	
}

string::string(const std::string& a_content)
: element(a_content, element_type::STRING)
{
}

string::~string()
{
	
}

/* object */

object::object()
: element("", element_type::OBJECT)
{
	
}

object::~object()
{
	
}

/* array */

array::array()
: element("", element_type::ARRAY)
{
	
}

array::~array()
{
	
}

/* master */

master::master()
{
	
}

master::~master()
{
	
}

/* casting helpers */

std::shared_ptr<object> as_object(std::shared_ptr<element> a_element) { return std::dynamic_pointer_cast<object>(a_element); }
std::shared_ptr<array> as_array(std::shared_ptr<element> a_element) { return std::dynamic_pointer_cast<array>(a_element); }
std::shared_ptr<string> as_string(std::shared_ptr<element> a_element) { return std::dynamic_pointer_cast<string>(a_element); }
std::shared_ptr<number> as_number(std::shared_ptr<element> a_element) { return std::dynamic_pointer_cast<number>(a_element); }
std::shared_ptr<constant> as_constant(std::shared_ptr<element> a_element) { return std::dynamic_pointer_cast<constant>(a_element); }

/* parse engine */

parse_engine::parse_engine(const std::string& a_json)
: m_json(a_json)
{
	
}

parse_engine::~parse_engine()
{
	
}

bool parse_engine::is_whitespace(char a_char)
{
	bool l_ret = false;
	if (
	(a_char == ' ') ||
	(a_char == '\n') ||
	(a_char == '\t')
	){
		l_ret = true;
	}
	return l_ret;
}

bool parse_engine::is_digit09(char a_char)
{
	return ((a_char >= 0x30) && (a_char <= 0x39));
}

bool parse_engine::is_digit19(char a_char)
{
	return ((a_char >= 0x31) && (a_char <= 0x39));
}

std::shared_ptr<string> parse_engine::collect_string()
{
	std::shared_ptr<string> l_ret = std::make_shared<string>();
	cs_state l_state = cs_state::CS_COLLECT_CHAR;
	std::string l_collected;
	while (m_json_it != m_json.end()) {
		char l_next = *m_json_it;
		switch (l_state) {
			case cs_state::CS_COLLECT_CHAR:
			{
				if (l_next == '\\') {
					auto l_bump_it = std::next(m_json_it, 1);
					char l_bump = *l_bump_it;
					if (l_bump == '\"') {
						// it's an excape sequence, so ignore it
						l_collected += l_next;
						l_collected += l_bump;
						++m_json_it; // extra increment because we used 2
					} else {
						// just a lonely backslash
						l_collected += l_next;
					}
				} else if (l_next == '\"') {
					l_ret->m_content = l_collected;
					goto cs_done;
				} else {
					l_collected += l_next;
				}
			}
			break;
		}
		++m_json_it;
	}
cs_done:
//	std::cerr << "collect_string collected: " << l_collected << std::endl;
	return l_ret;
}

std::shared_ptr<number> parse_engine::collect_number()
{
	std::shared_ptr<number> l_ret = std::make_shared<number>();
	cn_state l_state = cn_state::CN_COLLECT_FIRST;
	std::string l_collected;
	while (m_json_it != m_json.end()) {
		char l_next = *m_json_it;
		switch (l_state) {
			case cn_state::CN_COLLECT_FIRST:
			{
				// no need to be pedantic... worry about syntax when we convert to a numeric
				if ((is_digit09(l_next) || (l_next == '-') || (l_next == '+') || (l_next == 'e') || (l_next == 'E') || l_next == '.')) {
					l_collected += l_next;
				} else {
					// put back offending character
					--m_json_it;
					l_ret->m_content = l_collected;
					goto cn_done;
				}
			}
			break;
		}
		++m_json_it;
	}
cn_done:
	return l_ret;
}

std::shared_ptr<element> parse_engine::collect_value()
{
	std::shared_ptr<element> l_ret;
	cv_state l_state = cv_state::CV_IDENTIFY_VALUE;
	while (m_json_it != m_json.end()) {
		char l_next = *m_json_it;
		switch (l_state) {
			case cv_state::CV_IDENTIFY_VALUE:
			{
				if (is_whitespace(l_next)) {
					break;
				} else {
					if (l_next == '\"') {
						l_state = cv_state::CV_COLLECT_STRING;
						break;
					}
					if ((l_next == '-') || is_digit09(l_next)) {
						--m_json_it; // put it back
						l_state = cv_state::CV_COLLECT_NUMBER;
						break;
					}
					if (l_next == '{') {
						l_state = cv_state::CV_COLLECT_OBJECT;
						break;
					}
					if (l_next == '[') {
						l_state = cv_state::CV_COLLECT_ARRAY;
						break;
					}
					if (l_next == 'n') {
						// possible null
						++m_json_it;
						l_next = *m_json_it;
						if (l_next != 'u')
							break;
						++m_json_it;
						l_next = *m_json_it;
						if (l_next != 'l')
							break;
						++m_json_it;
						l_next = *m_json_it;
						if (l_next != 'l')
							break;
						// got null
						l_ret = std::make_shared<constant>(element_type::NULLELEMENT);
						goto cv_done;
					}
					if (l_next == 't') {
						// possible true
						++m_json_it;
						l_next = *m_json_it;
						if (l_next != 'r')
							break;
						++m_json_it;
						l_next = *m_json_it;
						if (l_next != 'u')
							break;
						++m_json_it;
						l_next = *m_json_it;
						if (l_next != 'e')
							break;
						// got true
						l_ret = std::make_shared<constant>(element_type::TRUE);
						goto cv_done;
					}
					if (l_next == 'f') {
						// possible false
						++m_json_it;
						l_next = *m_json_it;
						if (l_next != 'a')
							break;
						++m_json_it;
						l_next = *m_json_it;
						if (l_next != 'l')
							break;
						++m_json_it;
						l_next = *m_json_it;
						if (l_next != 's')
							break;
						++m_json_it;
						l_next = *m_json_it;
						if (l_next != 'e')
							break;
						// got false
						l_ret = std::make_shared<constant>(element_type::FALSE);
						goto cv_done;
					}
					// unexpected character
					std::stringstream l_ss;
					l_ss << "collect_value/CV_IDENTIFY_VALUE: unpexpected character: " << std::string(1, l_next) << " at position: " << std::distance(m_json.begin(), m_json_it);
					std::invalid_argument e(l_ss.str());
//					std::invalid_argument e("collect_value/CV_IDENTIFY_VALUE: unpexpected character: " + std::string(1, l_next));
					throw(e);
				}
			}
			break;
			case cv_state::CV_COLLECT_STRING:
			{
				l_ret = collect_string();
				goto cv_done;
				break;
			}
			break;
			case cv_state::CV_COLLECT_NUMBER:
			{
				l_ret = collect_number();
				goto cv_done;
				break;
			}
			break;
			case cv_state::CV_COLLECT_OBJECT:
			{
				l_ret = collect_object();
				goto cv_done;
				break;
			}
			break;
			case cv_state::CV_COLLECT_ARRAY:
			{
				l_ret = collect_array();
				goto cv_done;
				break;
			}
		}
		++m_json_it;
	}
cv_done:
	return l_ret;
}

std::shared_ptr<array> parse_engine::collect_array()
{
	std::shared_ptr<array> l_ret = std::make_shared<array>();
	ca_state l_state = ca_state::CA_FIND_VALUE;
	while (m_json_it != m_json.end()) {
		char l_next = *m_json_it;
		switch (l_state) {
			case ca_state::CA_FIND_VALUE:
			{
				if (is_whitespace(l_next)) {
					break;
				}
				if (l_next == ']') {
					// null array
					--m_json_it;
					l_state = ca_state::CA_FIND_TERMINATOR;
					break;
				}
				--m_json_it;
				l_state = ca_state::CA_COLLECT_VALUE;
			}
			break;
			case ca_state::CA_COLLECT_VALUE:
			{
				l_ret->values.push_back(collect_value());
				l_state = ca_state::CA_FIND_TERMINATOR;
			}
			break;
			case ca_state::CA_FIND_TERMINATOR:
			{
				if (is_whitespace(l_next)) {
					break;
				} else {
					if (l_next == ',') {
						l_state = ca_state::CA_FIND_VALUE;
						break;
					}
					if (l_next == ']') {
						goto ca_done;
						break;
					}
					// unexpected character
					std::stringstream l_ss;
					l_ss << "collect_array/CA_FIND_TERMINATOR: unpexpected character: " << std::string(1, l_next) << " at position: " << std::distance(m_json.begin(), m_json_it);
					std::invalid_argument e(l_ss.str());
//					std::invalid_argument e("collect_array/FIND_TERMINATOR: unpexpected character: " + std::string(1, l_next));
					throw(e);
				}
			}
			break;
		}
		++m_json_it;
	}
ca_done:
	return l_ret;
}

std::shared_ptr<object> parse_engine::collect_object()
{
	std::shared_ptr<object> l_ret = std::make_shared<object>();
	co_state l_state = co_state::CO_FIND_STRING;
	std::shared_ptr<string> l_curstring;
	std::shared_ptr<element> l_curvalue;
	while (m_json_it != m_json.end()) {
		char l_next = *m_json_it;
		switch (l_state) {
			case co_state::CO_FIND_STRING:
			{
				if (is_whitespace(l_next)) {
					break;
				} else {
					if (l_next == '\"') {
						l_state = co_state::CO_COLLECT_STRING;
						break;
					}
					if (l_next == '}') {
						goto co_done;
						break;
					}
					// unexpected character
					std::stringstream l_ss;
					l_ss << "collect_object/CO_FIND_STRING: unpexpected character: " << std::string(1, l_next) << " at position: " << std::distance(m_json.begin(), m_json_it);
					std::invalid_argument e(l_ss.str());
//					std::invalid_argument e("collect_object/FIND_STRING: unpexpected character: " + std::string(1, l_next));
					throw(e);
				}
			}
			break;
			case co_state::CO_COLLECT_STRING:
			{
				l_curstring = collect_string();
				l_state = co_state::CO_FIND_VALUE;
			}
			break;
			case co_state::CO_FIND_VALUE:
			{
				if (is_whitespace(l_next)) {
					break;
				} else {
					if (l_next == ':') {
						l_state = co_state::CO_COLLECT_VALUE;
						break;
					} else {
						// unexpected character
						std::stringstream l_ss;
						l_ss << "collect_object/CO_FIND_VALUE: unpexpected character: " << std::string(1, l_next) << " at position: " << std::distance(m_json.begin(), m_json_it);
						std::invalid_argument e(l_ss.str());
//						std::invalid_argument e("collect_object/FIND_VALUE: unpexpected character: " + std::string(1, l_next));
						throw(e);
					}
				}
			}
			break;
			case co_state::CO_COLLECT_VALUE:
			{
				l_curvalue = collect_value();
				l_state = co_state::CO_FIND_TERMINATOR;
			}
			break;
			case co_state::CO_FIND_TERMINATOR:
			{
				if (is_whitespace(l_next)) {
					break;
				} else {
					if (l_next == ',') {
						l_ret->stringvalues.insert(std::make_pair(l_curstring, l_curvalue));
						l_curstring.reset();
						l_curvalue.reset();
						l_state = co_state::CO_FIND_STRING;
						break;
					}
					if (l_next == '}') {
						l_ret->stringvalues.insert(std::make_pair(l_curstring, l_curvalue));
						l_curstring.reset();
						l_curvalue.reset();
						goto co_done;
						break;
					}
					// unexpected character
					std::stringstream l_ss;
					l_ss << "collect_object/CO_FIND_TERMINATOR: unpexpected character: " << std::string(1, l_next) << " at position: " << std::distance(m_json.begin(), m_json_it);
					std::invalid_argument e(l_ss.str());
//					std::invalid_argument e("collect_object/FIND_TERMINATOR: unpexpected character: " + std::string(1, l_next));
					throw(e);
				}
			}
			break;
		}
		++m_json_it;
	}
co_done:
	return l_ret;
}

std::shared_ptr<master> parse_engine::parse()
{
	m_master = std::make_shared<master>();
	parse_state l_state = parse_state::PS_FIND_MASTER;
	m_json_it = m_json.begin();
	while (m_json_it != m_json.end()) {
		char l_next = *m_json_it;
		switch (l_state) {
			case parse_state::PS_FIND_MASTER:
			{
				if (is_whitespace(l_next)) {
					break;
				} else {
					if (l_next == '{') {
						l_state = parse_state::PS_COLLECT_OBJECT;
						break;
					}
					if (l_next == '[') {
						l_state = parse_state::PS_COLLECT_ARRAY;
						break;
					}
				}
			}
			break;
			case parse_state::PS_COLLECT_OBJECT:
			{
				m_master->m_masterelement = collect_object();
				goto parse_done;
			}
			break;
			case parse_state::PS_COLLECT_ARRAY:
			{
				m_master->m_masterelement = collect_array();
				goto parse_done;
			}
			break;
		}
		++m_json_it;
	}
parse_done:
	return m_master;
}

/* parsing functions */

std::shared_ptr<master> parse_json(const std::string& a_json)
{
	parse_engine l_p(a_json);
	std::shared_ptr<master> l_ret = l_p.parse();
	return l_ret;
}

std::string make_human_readable(std::string a_json)
{
	// assumes a syntactically correct JSON string
	// tab/return on [ or { character,
	// reduce tab level + return on ] or } character
	std::size_t l_tablevel = 0;
	std::string l_ret;
	if (a_json.size() == 0)
		return l_ret;

	// remove all tabs, CR's and LF's
	std::string l_tonolf = a_json;
	std::string l_totrim;
	auto l_nolf_it = l_tonolf.begin();
	while (l_nolf_it != l_tonolf.end()) {
		switch (*l_nolf_it) {
			case '\t':
			case 10:
			case 13:
				// throw away
				break;
			default:
				l_totrim += *l_nolf_it;
		}
		++l_nolf_it;
	}

	// remove nearby spaces from brackets/braces/commas
	std::string l_trimback;
	bool l_inquotes = false;
	bool l_throwaway = false;

//	std::cout << "trimming: " << l_totrim << std::endl;
	auto l_fwd_it = l_totrim.begin();
	while (l_fwd_it != l_totrim.end()) {
		switch (*l_fwd_it) {
			case '{':
			case '}':
			case '[':
			case ']':
			case ',':
			case ':':
				if (!l_inquotes) {
					l_throwaway = true;
					l_trimback += *l_fwd_it;
				} else {
					l_trimback += *l_fwd_it;
				}
				break;
			case ' ':
				if (!l_throwaway) {
					l_trimback += *l_fwd_it;
				}
				break;
			case '"':
				l_inquotes = !l_inquotes;
				l_trimback += *l_fwd_it;
				break;
			case '\\':
				{
					// deal with escapes
					auto l_bump_it = std::next(l_fwd_it, 1);
					char l_next = *l_bump_it;
					if (l_next == '"') {
						// escaped quote character, ignore it
						l_trimback += *l_fwd_it;
						++l_fwd_it; // extra increment since we used 2 characters
						l_trimback += *l_fwd_it;
						break;
					} else {
						// just a rogue backslash, print it
						l_trimback += *l_fwd_it;
						break;
					}
					break;
				}
			default:
				l_trimback += *l_fwd_it;
				if (l_throwaway) {
					l_throwaway = false;
				}
				break;
		}
		++l_fwd_it;
	}

	std::string l_trimmed;
	l_throwaway = false;
	l_inquotes = false;

//	std::cout << "back trimming: " << l_trimback << std::endl;
	auto l_bwd_it = l_trimback.end();
	do {
		--l_bwd_it;
//		std::cout << " -" << *l_bwd_it << "- ";
//		std::cout.flush();
		switch (*l_bwd_it) {
			case '{':
			case '}':
			case '[':
			case ']':
			case ',':
			case ':':
				if (!l_inquotes) {
					l_throwaway = true;
					l_trimmed = *l_bwd_it + l_trimmed;
				} else {
					l_trimmed = *l_bwd_it + l_trimmed;
				}
				break;
			case ' ':
				if (!l_throwaway) {
					l_trimmed = *l_bwd_it + l_trimmed;
				}
				break;
			case '"':
				{
					// check if this is part of a backslash pair
					auto l_bump_it = std::next(l_bwd_it, -1);
					if (*l_bump_it == '\\') {
						l_trimmed = std::string(1, *l_bump_it) + std::string(1, *l_bwd_it) + l_trimmed;
						--l_bwd_it; // extra decrement since we used 2
					} else {
						l_inquotes = !l_inquotes;
						l_trimmed = *l_bwd_it + l_trimmed;
					}
					break;
				}
			default:
				l_trimmed = *l_bwd_it + l_trimmed;
				if (l_throwaway) {
					l_throwaway = false;
				}
				break;
		}
	} while (l_bwd_it != l_trimback.begin());
//	std::cout << "trimmed: " << l_trimmed << std::endl;

	auto embed_tabs = [&]() {
		for (std::size_t i = 1; i <= l_tablevel; ++i)
			l_ret += '\t';
	};
	
	l_inquotes = false;
	l_ret += '\n'; // kick it off with a line break
	auto l_lb_it = l_trimmed.begin();
	while (l_lb_it != l_trimmed.end()) {
		switch (*l_lb_it) {
			case '\\':
				{
					// skip over escaped quote if it exists
					auto l_bump_it = std::next(l_lb_it, 1);
					if (*l_bump_it == '"') {
						l_ret += *l_lb_it;
						++l_lb_it;
						l_ret += *l_lb_it;
						++l_lb_it;
						continue;
					} else {
						// just a solitary backslash
						l_ret += *l_lb_it;
						++l_lb_it;
						continue;
					}
					break;
				}
			case '\"':
				l_inquotes = !l_inquotes;
				break;
			case ':':
				if (!l_inquotes) {
					l_ret += ' ';
					l_ret += *l_lb_it;
					l_ret += ' ';
					++l_lb_it;
					continue;
				}
				break;
			case '{':
			case '[':
				if (!l_inquotes) {
					l_ret += *l_lb_it;
					l_ret += '\n';
					l_tablevel++;
					embed_tabs();
					++l_lb_it;
					continue;
				}
				break;
			case '}':
			case ']':
				if (!l_inquotes) {
					l_ret += '\n';
					l_tablevel--;
					embed_tabs();
					l_ret += *l_lb_it;
					++l_lb_it;
					continue;
				}
				break;
			case ',':
				if (!l_inquotes) {
					l_ret += *l_lb_it;
					l_ret += '\n';
					embed_tabs();
					++l_lb_it;
					continue;
				}
				break;
		}
		l_ret += *l_lb_it;
		++l_lb_it;
	}
	l_ret += '\n'; // line break at the end for good measure
	return l_ret;
}

/* serializer */

json_serializable::json_serializable(const std::string& a_object_name, element_type a_type)
: m_object_name(a_object_name)
, m_type(a_type)
{
}

json_serializable::~json_serializable()
{
	
}

std::string json_serializable::master_object_enclose(const std::string& a_json)
{
	return "{ " + a_json + " }";
}

std::string json_serializable::serialize()
{
	std::stringstream l_ret;
	if (m_type == ss::json::element_type::OBJECT) {
		l_ret << "\"" << object_name() << "\" : { ";
		l_ret << seal();
		l_ret << " }";
	} else if (m_type == ss::json::element_type::ARRAY) {
		l_ret << "[ ";
		l_ret << seal();
		l_ret << " ]";
	}
	return l_ret.str();
}

std::string json_serializable::stringvalue_number(const std::string& a_name, int a_num)
{
	std::stringstream l_ret;
	l_ret << "\"" << a_name << "\" : " << a_num;
	return l_ret.str();
}

std::string json_serializable::stringvalue_number(const std::string& a_name, double a_num)
{
	std::stringstream l_ret;
	l_ret << "\"" << a_name << "\" : " << a_num;
	return l_ret.str();
}

std::string json_serializable::stringvalue_string(const std::string& a_name, const std::string& a_string)
{
	std::stringstream l_ret;
	l_ret << "\"" << a_name << "\" : \"" << a_string << "\"";
	return l_ret.str();
}

std::string json_serializable::stringvalue_array(json_serializable& a_ser)
{
	std::stringstream l_ret;
	l_ret << "\"" << a_ser.object_name() << "\" : " << a_ser.serialize();
	return l_ret.str();
}

std::string json_serializable::value_number(int a_num)
{
	std::stringstream l_ret;
	l_ret << a_num;
	return l_ret.str();
}

std::string json_serializable::value_number(double a_num)
{
	std::stringstream l_ret;
	l_ret << a_num;
	return l_ret.str();
}

std::string json_serializable::value_string(const std::string& a_string)
{
	std::stringstream l_ret;
	l_ret << "\"" << a_string << "\"";
	return l_ret.str();
}

} // namespace json
} // namespace ss
