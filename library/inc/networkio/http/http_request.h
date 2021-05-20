
#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

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
#include <networkio/http/http_url.h>

#include <algorithm>
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
// networkio::http::request
//----------------------------------------------------------------------------

class request : public parser {

  public:
	request() : m_type("GET"), m_uri("/"), m_proto("HTTP"), m_version("1.1") {}

  public:
	const std::string &
	type() const {
		return this->m_type;
	}

	void
	set_type(std::string type) {
		this->m_type = std::move(type);
		std::transform(this->m_type.begin(), this->m_type.end(), this->m_type.begin(), ::toupper);
		this->update_header();
	}

	const std::string &
	uri() const {
		return this->m_uri;
	}

	void
	set_uri(std::string uri) {
		this->m_uri = std::move(uri);
		this->update_header();
	}

	void
	set_url(const std::string &url) {
		// TODO: parse
		// TODO: only encode get params
		// url_encode((char *)uri.c_str());
		printf("request::set_url(): not implemented yet.\n");
	}

	const std::string &
	proto() const {
		return this->m_proto;
	}

	void
	set_proto(std::string proto) {
		this->m_proto = std::move(proto);
		this->update_header();
	}

	std::unordered_map<std::string, std::string> &
	get_params() {
		return this->m_get_params;
	}

	std::unordered_map<std::string, std::string> &
	cookies() {
		return this->m_cookies;
	}

	parser_status from_string(const std::string &req);
	std::string to_string() const;

  protected:
	void update_header();
	std::unordered_map<std::string, std::string> parse_get_params(const std::string &s);
	std::unordered_map<std::string, std::string> parse_cookies(const std::string &s);

  protected:
	std::string m_type;
	std::string m_uri;
	std::unordered_map<std::string, std::string> m_get_params;
	std::string m_proto;
	std::string m_version;
	std::unordered_map<std::string, std::string> m_cookies;
};

} // namespace http
} // namespace networkio

#endif
