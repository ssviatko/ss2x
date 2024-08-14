#ifndef ND_H
#define ND_H

#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

#include "log.h"
#include "data.h"
#include "doubletime.h"
#include "ccl.h"

namespace ss {
namespace ccl {

class note {
	void copy_construct(const note& a_other_note);
	
public:

	// standard note names
	constexpr const static std::string SYS_DEFAULT = "SYS_DEFAULT";
	
	// standard replies
	constexpr const static std::string REPLY_OK = "REPLY_OK";
	constexpr const static std::string REPLY_CANCEL = "REPLY_CANCEL";
	constexpr const static std::string REPLY_ERROR = "REPLY_ERROR";
	
	// constructors
	note();
	note(const note& a_note);
	note(note&& a_note);
	note(const std::string& a_note_name);
	virtual ~note();
	
	// assignment
	note& operator=(const note& a_other_note);
	note& operator=(note&& a_other_note);

	// data access
	std::string name() const { return m_note_name; };
	std::string guid() const { return m_guid.as_hex_str_nospace(); }
	void set_delivered() { m_delivered = true; m_delivered_cond.notify_all(); }
	void set_seen() { m_seen = true; m_seen_cond.notify_all(); }
	void set_reply_requested() { m_reply_requested = true; }
	void set_replied() { m_replied = true; m_replied_cond.notify_all(); }
	void set_reply(const std::string& a_reply);
	
	bool delivered() const { return m_delivered; }
	bool seen() const { return m_seen; }
	bool reply_requested() const { return m_reply_requested; }
	bool replied() const { return m_replied; }
	std::string reply();
	
	bool wait_for_delivered(std::size_t a_timeout_ms);
	bool wait_for_seen(std::size_t a_timeout_ms);
	bool wait_for_replied(std::size_t a_timeout_ms);
	
protected:
	std::string m_note_name;
	std::string m_reply_name;
	std::mutex m_reply_name_mutex;
	data m_guid;
	std::atomic<bool> m_delivered;
	std::mutex m_delivered_mutex;
	std::condition_variable m_delivered_cond;
	std::atomic<bool> m_seen;
	std::mutex m_seen_mutex;
	std::condition_variable m_seen_cond;
	std::atomic<bool> m_reply_requested;
	std::atomic<bool> m_replied;
	std::mutex m_replied_mutex;
	std::condition_variable m_replied_cond;
};

} // namespace ccl
} // namespace ss

#endif //ND_H

