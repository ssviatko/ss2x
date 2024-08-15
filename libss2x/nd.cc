#include "nd.h"

namespace ss {
namespace ccl {

/* note_attributes */

void note_attributes::set_keyvalue(const std::string& a_key, const std::string& a_value)
{
	m_attribdb.insert(std::pair<std::string, std::string>(a_key, a_value));
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

/* nd (note dispatcher) */

nd::nd()
: ss::ccl::dispatchable("nd")
{
	start();
}

nd::~nd()
{
	shutdown();
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
	ctx.log(std::format("dispatch: got note name={} guid={} attribs={}", l_note.value().name(), l_note.value().guid(), l_note.value().attributes().size()));
	std::string l_guid = l_note.value().guid();
	m_notedb.insert(std::pair<std::string, ss::ccl::note>(l_guid, l_note.value()));
	return true;
}

std::string nd::post(const std::string& a_note_name, ss::ccl::note_attributes a_attributes)
{
	ss::ccl::note l_new_note(a_note_name);
	l_new_note.set_attributes(a_attributes);
	m_post_queue.add_work_item(l_new_note);
	return l_new_note.guid();
}

} // namespace ccl
} // namespace ss
