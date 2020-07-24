
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
	request(void) : m_type("GET"), m_uri("/"), m_proto("HTTP"), m_version("1.1") {}

  public:
	const std::string &
	type(void) const {
		return this->m_type;
	}

	void
	set_type(std::string type) {
		this->m_type = type;
		std::transform(this->m_type.begin(), this->m_type.end(), this->m_type.begin(), ::toupper);
	}

	const std::string &
	uri(void) const {
		return this->m_uri;
	}

	void
	set_uri(std::string uri) {
		this->m_uri = uri;
	}

	void
	set_url(std::string url) {
		// TODO: parse
		// TODO: only encode get params
		// url_encode((char *)uri.c_str());
		printf("request::set_url(): not implemented yet.\n");
	}

	std::unordered_map<std::string, std::string> &
	get_params(void) {
		return this->m_get_params;
	}

	std::unordered_map<std::string, std::string> &
	cookies(void) {
		return this->m_cookies;
	}

	parser_status from_string(const std::string &req);
	std::string to_string(void);

  protected:
	std::unordered_map<std::string, std::string> parse_get_params(const std::string s);
	std::unordered_map<std::string, std::string> parse_cookies(const std::string s);

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
