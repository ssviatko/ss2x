#include "nd.h"

namespace ss {
namespace ccl {

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
}

void note::set_reply(const std::string& a_reply)
{
	std::lock_guard<std::mutex> l_guard(m_reply_name_mutex);
	m_reply_name = a_reply;
	set_replied();
}

std::string note::reply()
{
	std::lock_guard<std::mutex> l_guard(m_reply_name_mutex);
	return m_reply_name;
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

} // namespace ccl
} // namespace ss
