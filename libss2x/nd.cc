#include "nd.h"

namespace ss {
namespace ccl {

/* note_attributes */

void note_attributes::set_keyvalue(const std::string& a_key, const std::string& a_value)
{
	m_attribdb.insert(std::pair<std::string, std::string>(a_key, a_value));
}

std::string note_attributes::keyvalue(const std::string& a_key)
{
	std::string l_ret;
	l_ret = m_attribdb.at(a_key);
	return l_ret;
}

/* note */

note::note()
: m_note_name("none")
, m_reply_name("none")
, m_delivered(false)
, m_seen(false)
, m_reply_requested(false)
, m_replied(false)
{
	m_guid.random(16);
}

note::note(const std::string& a_note_name)
: m_note_name(a_note_name)
, m_reply_name("none")
, m_delivered(false)
, m_seen(false)
, m_reply_requested(false)
, m_replied(false)
{
	m_guid.random(16);
}

note::note(const note& a_note)
{
	copy_construct(a_note);
}

note::note(note&& a_note)
{
	copy_construct(a_note);
}

note& note::operator=(const note& a_other_note)
{
	copy_construct(a_other_note);
	return *this;
}

note& note::operator=(note&& a_other_note)
{
	copy_construct(a_other_note);
	return *this;
}

note::~note()
{
	
}

void note::copy_construct(const note& a_other_note)
{
	m_note_name = a_other_note.m_note_name;
	m_reply_name_mutex.lock();
	m_reply_name = a_other_note.m_reply_name;
	m_reply_name_mutex.unlock();
	m_guid = a_other_note.m_guid;
	m_delivered = a_other_note.m_delivered.load();
	m_seen = a_other_note.m_seen.load();
	m_reply_requested = a_other_note.m_reply_requested.load();
	m_replied = a_other_note.m_replied.load();
	m_attributes_mutex.lock();
	m_attributes = a_other_note.m_attributes;
	m_attributes_mutex.unlock();
}

void note::set_reply(const std::string& a_reply)
{
	std::lock_guard<std::mutex> l_guard(m_reply_name_mutex);
	m_reply_name = a_reply;
	set_replied();
}

void note::set_attributes(const note_attributes& a_attrib)
{
	std::lock_guard<std::mutex> l_guard(m_attributes_mutex);
	m_attributes = a_attrib;
}

std::string note::reply()
{
	std::lock_guard<std::mutex> l_guard(m_reply_name_mutex);
	return m_reply_name;
}

note_attributes note::attributes()
{
	std::lock_guard<std::mutex> l_guard(m_attributes_mutex);
	return m_attributes;
}

bool note::wait_for_delivered(std::size_t a_timeout_ms)
{
	if (m_delivered)
		return true;
	std::unique_lock l_ul(m_delivered_mutex);
	if (m_delivered_cond.wait_for(l_ul, std::chrono::milliseconds(a_timeout_ms)) == std::cv_status::timeout) {
		l_ul.unlock();
		return false;
	}
	return true;
}

bool note::wait_for_seen(std::size_t a_timeout_ms)
{
	if (m_seen)
		return true;
	std::unique_lock l_ul(m_seen_mutex);
	if (m_seen_cond.wait_for(l_ul, std::chrono::milliseconds(a_timeout_ms)) == std::cv_status::timeout) {
		l_ul.unlock();
		return false;
	}
	return true;
}

bool note::wait_for_replied(std::size_t a_timeout_ms)
{
	if (m_replied)
		return true;
	std::unique_lock l_ul(m_replied_mutex);
	if (m_replied_cond.wait_for(l_ul, std::chrono::milliseconds(a_timeout_ms)) == std::cv_status::timeout) {
		l_ul.unlock();
		return false;
	}
	return true;
}

/* nd_agent */

void nd_agent::dispatch(std::string a_work_item)
{
	ctx.log(std::format("queue_worker: got string {}", a_work_item));
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

nd_agent::~nd_agent()
{
}

/* nd (note dispatcher) */

nd::nd()
: ss::ccl::dispatchable("nd")
{
	start();
	for (std::size_t i = 0; i < AGENTS; ++i) {
		std::stringstream l_name;
		l_name << "nd_agent_" << i;
		std::shared_ptr<nd_agent> l_agent = std::make_shared<nd_agent>(l_name.str(), m_call_queue);
		l_agent->start();
		m_agents.push_back(l_agent);
	}
}

nd::~nd()
{
}

nd& nd::get()
{
	static nd shared_instance;
	return shared_instance;
}

void nd::starting()
{
	ctx.log("note dispatcher: start request received...");
}

void nd::started()
{
	ctx.log("note dispatcher started.");
}

void nd::shutdown()
{
	halt();
	m_call_queue.shut_down();
	for (const auto& i : m_agents) {
		i->join();
	}
}

void nd::halting()
{
	ctx.log("note dipatcher: halt request received....");
}

void nd::halted()
{
	ctx.log("note dispatcher: halted.");
}

bool nd::dispatch()
{
	ctx.log("dispatch: waiting for note");
	std::optional<ss::ccl::note> l_note = m_post_queue.wait_for_item(20);
	if (!l_note.has_value())
		return true;
	ss::ccl::note l_work = l_note.value();
	ctx.log(std::format("dispatch: got note name={} guid={} attribs={}", l_work.name(), l_work.guid(), l_work.attributes().size()));
	note_attributes l_attrib = l_work.attributes();
	for (const auto& [ key, value ] : l_attrib.keymap()) {
		ctx.log(std::format("key={} value={}", key, value));
	}
	std::string l_guid = l_work.guid();
	m_notedb.insert(std::pair<std::string, ss::ccl::note>(l_guid, l_work));
	m_call_queue.add_work_item(l_guid);
	return true;
}

std::string nd::post(const std::string& a_note_name, bool a_reply, ss::ccl::note_attributes a_attributes)
{
	std::lock_guard<std::mutex> l_guard(m_notedb_mutex);
	ss::ccl::note l_new_note(a_note_name);
	if (a_reply)
		l_new_note.set_reply_requested();
	l_new_note.set_attributes(a_attributes);
	m_post_queue.add_work_item(l_new_note);
	return l_new_note.guid();
}

void nd::dispose(const std::string a_guid)
{
	std::lock_guard<std::mutex> l_guard(m_notedb_mutex);
	m_notedb.erase(a_guid);
//	ctx.log(std::format("m_notedb now has {} items.", m_notedb.size()));
}

} // namespace ccl
} // namespace ss
