
#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/fnv1a.h>
#include <networkio/interfaces/server.h>
#include <networkio/proto/string_proto.h>
#include <networkio/socket/concurrent_server.h>
#include <networkio/socket/socket.h>
#include <networkio/socket/tcp.h>
#include <networkio/stringutil.h>
#include <networkio/types.h>

#include <networkio/http/http_parser.h>
#include <networkio/http/http_status.h>

#include <atomic>
#include <deque>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>


//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace http {

//----------------------------------------------------------------------------
// networkio::http::response_cookie
//----------------------------------------------------------------------------

class response_cookie {

  public:
	response_cookie();

	//
	// TODO: validate/sanitize on set
	//
	const std::string &
	name() const {
		return this->m_name;
	}

	void
	set_name(const std::string &s) {
		this->m_name = s;
	}

	const std::string &
	value() const {
		return this->m_value;
	}

	void
	set_value(const std::string &s) {
		this->m_value = s;
	}

	const std::string &
	expire_date() const {
		return this->m_expire_date;
	}

	void
	set_expire_date(const std::string &s) {
		this->m_expire_date = s;
	}

	int32_t
	max_age() const {
		return this->m_max_age;
	}

	void
	set_expire_date(int32_t i) {
		this->m_max_age = i;
	}

	const std::string &
	domain() const {
		return this->m_domain;
	}

	void
	set_domain(const std::string &s) {
		this->m_domain = s;
	}

	const std::string &
	path() const {
		return this->m_path;
	}

	void
	set_path(const std::string &s) {
		this->m_path = s;
	}

	bool
	secure() const {
		return this->m_secure;
	}

	void
	set_secure(bool b) {
		this->m_secure = b;
	}

	bool
	http_only() const {
		return this->m_http_only;
	}

	void
	set_http_only(bool b) {
		this->m_http_only = b;
	}

	const std::string &
	same_site() const {
		return this->m_same_site;
	}

	void
	set_same_site(const std::string &s) {
		this->m_same_site = s;
	}

	bool from_string(const std::string &c);
	std::string to_string();

  protected:
	std::string m_name;
	std::string m_value;
	std::string m_expire_date;
	int32_t m_max_age = -1;
	std::string m_domain;
	std::string m_path;
	bool m_secure = false;
	bool m_http_only = false;
	std::string m_same_site; // TODO: enum
	int m_version = 0;
};

//----------------------------------------------------------------------------
// networkio::http::response
//----------------------------------------------------------------------------

class response : public parser {

  public:
	response() : m_proto("HTTP"), m_version("1.1") {
		this->set_status(404);
		this->m_header_fields["server"] = "apache";
		this->m_header_fields["content-type"] = "text/html"; // default
		this->m_header_fields["content-length"] = "0";
	}

	response(int32_t status) : m_proto("HTTP"), m_version("1.1") {
		this->set_status(status);
		this->m_header_fields["server"] = "apache";
		this->m_header_fields["content-type"] = "text/html";
		this->m_header_fields["content-length"] = "0";
	}

	response(const std::string &body) : m_proto("HTTP"), m_version("1.1") {
		this->set_status(200);
		this->m_header_fields["server"] = "apache";
		this->m_header_fields["content-type"] = "text/html";
		this->m_header_fields["content-length"] = "0";
		this->set_body(body);
	}

  public:
	virtual void
	set_body(const std::string &s) {
		this->set_status(200);
		this->m_header_fields["content-length"] = std::to_string(s.length());
		this->m_body = s;
	}

	int32_t
	status() {
		return this->m_status;
	}

	void
	set_status(int32_t s) {
		this->m_status = s;
		this->m_status_str = status_code_to_str(s);
	}

	std::unordered_map<std::string, response_cookie> &
	cookies() {
		return this->m_cookies;
	}

	// TODO: would be cooler if this was an actual contructor
	bool set_file(const std::string &f);

	parser_status from_string(std::string &resp);
	std::string to_string();

  protected:
	std::string m_proto;
	std::string m_version;
	int32_t m_status;
	std::string m_status_str;
	std::unordered_map<std::string, response_cookie> m_cookies;
};

} // namespace http
} // namespace networkio

#endif
