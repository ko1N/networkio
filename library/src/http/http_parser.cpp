//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <algorithm>
#include <string>
#include <vector>

#include <networkio/http/http_parser.h>
#include <networkio/stringutil.h>

using namespace networkio;
using namespace networkio::http;

//----------------------------------------------------------------------------
// networkio::http::parser
//----------------------------------------------------------------------------

auto parser::header_from_string(const std::string &http) -> parser_status {
  // seperate body from header
  auto header_end = http.find("\r\n\r\n");
  if (header_end == std::string::npos) {
    // printf("http::parser::from_string(): unable to find http header end
    // marker.\n");
    return PARSE_ERROR_AGAIN;
  }

  // TODO: is it efficient to copy the content here?
  std::string header = http.substr(0, header_end);
  this->m_body = http.substr(header_end + 4);

  // parse header
  auto header_lines = std::tokenize(header, "\r\n");
  if (header_lines.empty()) {
    printf("http::parser::from_string(): header only consists of 1 line.\n");
    return PARSE_ERROR_AGAIN;
  }

  this->m_header = header_lines[0];

  for (size_t i = 1; i < header_lines.size(); i++) {
    auto &l = header_lines[i];
    auto d = l.find(':');
    if (d == std::string::npos) {
      break; // something went wrong?
    }

    std::string key = l.substr(0, d);
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    std::string val = l.substr(d + 2);
    this->m_header_fields[key] = val;
  }

  return PARSE_SUCCESS;
}

auto parser::header_to_string() const -> std::string {
  // header
  std::string r = this->m_header + "\r\n";

  // header fields
  for (const auto &f : this->m_header_fields) {
    r += f.first + ": " + f.second += "\r\n";
  }

  r += "\r\n";

  // body
  r += this->m_body;

  return r;
}
