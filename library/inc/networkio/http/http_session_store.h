#ifndef __HTTP_SESSION_STORE_H__
#define __HTTP_SESSION_STORE_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include <networkio/http/http_server.h>
#include <networkio/memory/user_data.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace http {

//----------------------------------------------------------------------------
// networkio::http::session
//----------------------------------------------------------------------------

class session {

  public:
	session();
	session(const std::string &sid);

  public:
	bool
	is_valid() {
		// TODO: check for timeout
		return this->m_session_id != "";
	}

	const std::string &
	session_id() {
		return this->m_session_id;
	}

	// see mux::client
	// thread safe userdata
	// TODO: refactor using constexpr - where iss the thread safety here?
	template <typename T>
	std::shared_ptr<T>
	get_userdata(const std::string &userdata) {
		if (this->m_userdata[userdata] == nullptr) {
			this->m_userdata[userdata].reset(new T());
		}

		return std::static_pointer_cast<T>(this->m_userdata[userdata]);
	}

	template <typename T>
	std::shared_ptr<T>
	get_userdata(const std::string &userdata, std::function<T *()> &&func) {
		if (this->m_userdata[userdata] == nullptr) {
			this->m_userdata[userdata].reset(func());
		}

		return std::static_pointer_cast<T>(this->m_userdata[userdata]);
	}

  protected:
	std::string m_session_id;
	std::unordered_map<std::string, std::shared_ptr<networkio::memory::base_userdata>> m_userdata;
};

//----------------------------------------------------------------------------
// networkio::http::session_store
//----------------------------------------------------------------------------

// TODO: singleton or one  per server
class session_store {

  public:
	session_store();

  public:
	std::shared_ptr<session> get(connection &conn); // TODO: figure out a way to make it const

  protected:
	std::string generate_session_id();

  protected:
	std::shared_mutex m_mutex;
	std::unordered_map<std::string, std::shared_ptr<session>> m_sessions;
};

} // namespace http
} // namespace networkio

#endif
