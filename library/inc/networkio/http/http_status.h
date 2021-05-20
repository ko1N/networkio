#ifndef HTTP_STATUS_H_
#define HTTP_STATUS_H_

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace http {

static struct {
  int32_t code;
  const char *name;
} status_codes[] = {{100, "Continue"},
                    {101, "Switching Protocols"},
                    {200, "OK"},
                    {201, "Created"},
                    {202, "Accepted"},
                    {203, "Non-Authoritative Information"},
                    {204, "No Content"},
                    {205, "Reset Content"},
                    {206, "Partial Content"},
                    {300, "Multiple Choices"},
                    {302, "Found"},
                    {303, "See Other"},
                    {304, "Not Modified"},
                    {305, "Use Proxy"},
                    {400, "Bad Request"},
                    {401, "Unauthorized"},
                    {402, "Payment Required"},
                    {403, "Forbidden"},
                    {404, "Not Found"},
                    {405, "Method Not Allowed"},
                    {406, "Not Acceptable"},
                    {407, "Proxy Authentication Required"},
                    {408, "Request Timeout"},
                    {409, "Conflict"},
                    {410, "Gone"},
                    {411, "Length Required"},
                    {412, "Precondition Failed"},
                    {413, "Request Entity Too Large"},
                    {414, "Request-URI Too Long"},
                    {415, "Unsupported Media Type"},
                    {416, "Requested Range Not Satisfiable"},
                    {417, "Expectation Failed"},
                    {500, "Internal Server Error"},
                    {501, "Not Implemented"},
                    {502, "Bad Gateway"},
                    {503, "Service Unavailable"},
                    {504, "Gateway Timeout"},
                    {505, "HTTP Version Not Supported"},
                    {0, nullptr}};

//
// TODO:
// setup hashmap to speed up lookup and reverse lookup process
//

static inline std::string status_code_to_str(int32_t code) {
  for (int i = 0; status_codes[i].code != 0; i++) {
    if (status_codes[i].code == code) {
      return status_codes[i].name;
    }
  }
  return "Unknown";
}

static inline int32_t status_str_to_code(const std::string &name) {
  for (int i = 0; status_codes[i].code != 0; i++) {
    if (status_codes[i].name == name) {
      return status_codes[i].code;
    }
  }
  return -1;
}

static inline std::string resolve_content_type(const std::string &filename) {
  std::string ext;
  try {
    ext = filename.substr(filename.find_last_of('.') + 1);
  } catch (std::out_of_range &e) {
    return "text/plain";
  }

  if (ext == "png") {
    return "image/png";
  } else if (ext == "jpg" || ext == "jpeg") {
    return "image/jpeg";
  } else if (ext == "ico") {
    return "image/x-icon";
  } else if (ext == "css") {
    return "text/css";
  } else if (ext == "htm" || ext == "html") {
    return "text/html";
  }

  return "text/plain";
}

} // namespace http
} // namespace networkio

#endif