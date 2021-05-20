#ifndef HTTP_PARSER_H_
#define HTTP_PARSER_H_

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
  PARSE_SUCCESS = 0,     // parse success
  PARSE_ERROR = -1,      // general parse error
  PARSE_ERROR_AGAIN = -2 // not enough data provided
};

class parser {
public:
  parser() = default;
  virtual ~parser() = default;

  auto header() -> const std::string & { return this->m_header; }

  virtual void set_header(const std::string &s) { this->m_header = s; }

  // lookups are not const
  auto header_fields() -> std::unordered_map<std::string, std::string> & {
    return this->m_header_fields;
  }

  auto body() -> const std::string & { return this->m_body; }

  virtual void set_body(const std::string &s) { this->m_body = s; }

protected:
  auto header_from_string(const std::string &http) -> parser_status;
  auto header_to_string() const -> std::string;

  std::string m_header;
  std::unordered_map<std::string, std::string> m_header_fields;
  std::string m_body;
};

} // namespace http
} // namespace networkio

#endif