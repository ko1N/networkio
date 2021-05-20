
//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include <algorithm>
#include <fstream>
#include <list>

#include <networkio/http/http_request.h>

using namespace networkio;
using namespace networkio::http;
using namespace networkio::socket;
using namespace networkio::socket::tcp;

//----------------------------------------------------------------------------
// networkio::http::request
//----------------------------------------------------------------------------

parser_status request::from_string(const std::string &req) {
  auto status = this->header_from_string(req);
  if (status != PARSE_SUCCESS) {
    if (status == PARSE_ERROR) {
      printf("http::request::from_string(): unable to parse request.\n");
    }
    return status;
  }

  // parse request line in header
  auto header_tokens = std::tokenize(this->m_header, ' ');
  if (header_tokens.size() < 3) {
    printf(
        "http::request::from_string(): invalid header line in request:\n%s\n\n",
        req.c_str());
    return PARSE_ERROR;
  }

  // parse type (GET, POST, PUT, etc)
  this->m_type = header_tokens[0];

  // parse uri
  // TODO: does really only GET use get params?
  if (this->m_type == "GET") {
    std::string uri = url_decode((char *)header_tokens[1].c_str());

    // TODO: empty string should be added at the end?
    auto params = std::tokenize(uri, '?');
    if (params.empty()) {
      this->m_uri = uri;
      this->m_get_params.clear();
    } else {
      this->m_uri = params[0];
      if (params.size() > 1) {
        this->m_get_params = this->parse_get_params(params[1]);
      }
    }
  } else {
    this->m_uri = url_decode((char *)header_tokens[1].c_str());
    this->m_get_params.clear();
  }

  // parse protocol
  auto proto = std::tokenize(header_tokens[2], '/');
  if (proto.size() != 2) {
    printf("http::request::from_string(): invalid protocol line \"%s\".\n",
           header_tokens[2].c_str());
    return PARSE_ERROR;
  }

  this->m_proto = proto[0];
  this->m_version = proto[1];

  // check for content length
  if (this->m_header_fields["content-length"] != "") {
    try {
      int content_length = std::stoi(this->m_header_fields["content-length"]);
      if (content_length > 0 &&
          this->m_body.length() < (size_t)content_length) {
        // printf("http::request::from_string(): response not finished yet %d <
        // %d.\n", (int)this->m_body.length(), content_length);
        return PARSE_ERROR_AGAIN;
      }
    } catch (std::out_of_range &e) {
    }
  }

  // check for form data
  if (this->m_header_fields["content-type"] ==
      "application/x-www-form-urlencoded") {
    // is it ok to override get params here?
    this->m_get_params =
        this->parse_get_params(this->m_body); // TODO: rename to params
  }

  if (this->m_header_fields["cookie"] != "") {
    this->m_cookies = this->parse_cookies(this->m_header_fields["cookie"]);
  }

  return PARSE_SUCCESS;
}

std::string request::to_string() const {
  // TODO: get parameters...
  // TODO: write cookies!
  return this->header_to_string();
}

void request::update_header() {
  this->m_header = this->m_type + " " + // TODO: sanitize on set
                   this->m_uri + " " +  // TODO: sanitize on set
                   this->m_proto + "/" + this->m_version;
}

std::unordered_map<std::string, std::string>
request::parse_get_params(const std::string &s) {
  std::unordered_map<std::string, std::string> res;

  std::vector<std::string> pairs = std::tokenize(s, '&');
  for (size_t i = 0; i < pairs.size(); i++) {
    std::vector<std::string> tokens = std::tokenize(pairs[i], '=');
    if (tokens.size() == 2) {
      res[tokens[0]] = tokens[1];
    }
  }

  return res;
}

std::unordered_map<std::string, std::string>
request::parse_cookies(const std::string &s) {
  std::unordered_map<std::string, std::string> r;

  auto cookies = std::tokenize(s, ';');
  for (size_t i = 0; i < cookies.size(); i++) {
    auto cookie = std::tokenize(cookies[i], '=');
    if (cookie.size() == 2) {
      /*
      A <cookie-name> can be any US-ASCII characters except control characters
      (CTLs), spaces, or tabs. It also must not contain a separator character
      like the following: ( ) < > @ , ; : \ " /  [ ] ? = { }.
      */
      // TODO: we should reject malformed cookies here
      std::string key = std::strip_all(cookie[0], " \t()<>@,;:\\\"/[]?={}");
      std::string val = url_decode((char *)cookie[1].c_str());
      r[key] = val;
    } else {
      printf("http::request::parse_cookies(): invalid cookie found \"%s\".\n",
             cookies[i].c_str());
    }
  }

  return r;
}