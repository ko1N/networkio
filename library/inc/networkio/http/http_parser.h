#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <string>
#include <unordered_map>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace http {

/*
-1 fail
0 not finished
1 finished
*/
enum parser_status {
	PARSE_SUCCESS = 0,	   // parse success
	PARSE_ERROR = -1,	   // general parse error
	PARSE_ERROR_AGAIN = -2 // not enough data provided
};

class parser {
  public:
	parser(void);
	virtual ~parser();

	const std::string &
	header(void) {
		return this->m_header;
	}

	virtual void
	set_header(const std::string &s) {
		this->m_header = s;
	}

	// lookups are not const
	std::unordered_map<std::string, std::string> &
	header_fields(void) {
		return this->m_header_fields;
	}

	const std::string &
	body(void) {
		return this->m_body;
	}

	virtual void
	set_body(const std::string &s) {
		this->m_body = s;
	}

  protected:
	parser_status header_from_string(std::string http);
	std::string header_to_string(void);

  protected:
	std::string m_header;
	std::unordered_map<std::string, std::string> m_header_fields;
	std::string m_body;
};

} // namespace http
} // namespace networkio

#endif