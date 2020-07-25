//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <algorithm>
#include <fstream>
#include <list>

#include <networkio/http/http_response.h>
#include <networkio/http/http_url.h>

using namespace networkio;
using namespace networkio::http;
using namespace networkio::socket;
using namespace networkio::socket::tcp;

//----------------------------------------------------------------------------
// networkio::http::response_cookie
//----------------------------------------------------------------------------

response_cookie::response_cookie(void) {}

bool
response_cookie::from_string(const std::string &c) {
	/*
	name="value with spaces";Max-Age=3600;Path=/;Version=1
	name=value%20with%20spaces;Expires=Mon, 29-Aug-2011 14:30:00 GMT;Path=/
	Set-Cookie: id=a3fWa; Expires=Wed, 21 Oct 2015 07:28:00 GMT; Secure; HttpOnly
	*/
	auto flags = std::tokenize(c, ';');
	for (size_t i = 0; i < flags.size(); i++) {
		auto flag = std::tokenize(flags[i], '=');
		if (flag.size() > 0) {
			// TODO: make a sanitize_cookie key/value function globally
			/*
			A <cookie-name> can be any US-ASCII characters except control characters (CTLs), spaces, or tabs.
			It also must not contain a separator character like the following: ( ) < > @ , ; : \ " /  [ ] ? = { }.
			*/
			// TODO: we should reject malformed cookies here
			std::string key = std::strip_all(flag[0], " \t()<>@,;:\\\"/[]?={}");

			if (i == 0) {
				// cookie name
				this->m_name = key;
				std::string val = flag[1];
				if (val.size() > 2 && val[0] == '\"' && val[val.size() - 1] == '\"') {
					this->m_value = val.substr(1, val.size() - 2);
				} else {
					this->m_value = url_decode((char *)val.c_str());
				}
			} else {
				std::transform(key.begin(), key.end(), key.begin(), ::tolower);
				if (key == "expires") {
					if (flag.size() < 2) {
						printf("http::response_cookie::from_string(): invalid Expires value.\n");
						return false;
					}
					this->m_expire_date = flag[1];
				} else if (key == "max-age") {
					if (flag.size() < 2) {
						printf("http::response_cookie::from_string(): invalid Max-Age value.\n");
						return false;
					}
					try {
						this->m_max_age = std::stoi(flag[1]);
					} catch (std::out_of_range &e) {
						printf("http::response_cookie::from_string(): unable to parse Max-Age value \"%s\".\n",
							   flag[1].c_str());
					}
				} else if (key == "domain") {
					if (flag.size() < 2) {
						printf("http::response_cookie::from_string(): invalid Domain value.\n");
						return false;
					}
					this->m_domain = flag[1];
				} else if (key == "path") {
					if (flag.size() < 2) {
						printf("http::response_cookie::from_string(): invalid Path value.\n");
						return false;
					}
					this->m_path = flag[1];
				} else if (key == "secure") {
					this->m_secure = true;
				} else if (key == "httponly") {
					this->m_http_only = true;
				} else if (key == "samesite") {
					if (flag.size() < 2) {
						printf("http::response_cookie::from_string(): invalid SameSite value.\n");
						return false;
					}
				} else if (key == "version") {
					if (flag.size() < 2) {
						printf("http::response_cookie::from_string(): invalid Version value.\n");
						return false;
					}
					try {
						this->m_version = std::stoi(flag[1]);
					} catch (std::out_of_range &e) {
						printf("http::response_cookie::from_string(): unable to parse Version value \"%s\".\n",
							   flag[1].c_str());
					}
				}
			}
		} else {
			printf("http::response_cookie::from_string(): invalid flag found \"%s\".\n", flags[i].c_str());
			return false;
		}
	}

	// check cookie version
	switch (this->m_version) {
		case 1: {
			if (this->m_value.size() > 2 && this->m_value[0] == '\"' &&
				this->m_value[this->m_value.size() - 1] == '\"') {
				this->m_value = this->m_value.substr(1, this->m_value.size() - 2);
			} else {
				printf("http::response_cookie::from_string(): cookie version=1 with invalid value \"%s\".\n",
					   this->m_value.c_str());
				return false;
			}
		} break;

		default: {
			this->m_value = url_decode((char *)this->m_value.c_str());
		} break;
	}

	return true;
}

/*
name=value%20with%20spaces;Expires=Mon, 29-Aug-2011 14:30:00 GMT;Path=/
Set-Cookie: id=a3fWa; Expires=Wed, 21 Oct 2015 07:28:00 GMT; Secure; HttpOnly
*/
std::string
response_cookie::to_string(void) {
	std::string r;

	r += this->m_name + "=" + url_encode((char *)this->m_value.c_str());
	if (this->m_expire_date != "") {
		r += ";Expires=" + this->m_expire_date;
	}
	if (this->m_max_age >= 0) {
		r += ";Max-Age=" + std::to_string(this->m_max_age);
	}
	if (this->m_domain != "") {
		r += ";Domain=" + this->m_domain;
	}
	if (this->m_path != "") {
		r += ";Path=" + this->m_path;
	}
	if (this->m_secure) {
		r += ";Secure";
	}
	if (this->m_http_only) {
		r += ";HttpOnly";
	}
	if (this->m_same_site != "") {
		r += ";SameSite=" + this->m_same_site;
	}
	return r;
}

//----------------------------------------------------------------------------
// networkio::http::response
//----------------------------------------------------------------------------

size_t
file_getsize(std::string f) {
	std::ifstream in(f, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}

bool
file_read(std::string f, void *buf, size_t size) {
	if (size != file_getsize(f)) {
		return false;
	}

	std::ifstream ifile(f, std::ios::in | std::ios::binary);
	if (ifile.is_open()) {
		ifile.read((char *)buf, size);
		ifile.close();
		return true;
	}

	return false;
}

bool
response::set_file(std::string f) {
	size_t size = file_getsize(f);
	if (size == 0) {
		this->set_status(404);
		this->m_body = "";
		return false;
	}

	void *buf = malloc(size);
	if (file_read(f, buf, size)) {
		this->set_status(200);
		this->m_header_fields["content-type"] = resolve_content_type(f);
		this->m_body = std::string((char *)buf, size);
		free(buf);
		return true;
	} else {
		this->set_status(404);
		this->m_body = "";
		free(buf);
		return false;
	}
}

parser_status
response::from_string(std::string &resp) {
	auto status = this->header_from_string(resp);
	if (status != PARSE_SUCCESS) {
		// printf("http::response::from_string(): unable to parse response.\n");
		return status;
	}

	// parse response line in header
	auto header_tokens = std::tokenize(this->m_header, ' ');
	if (header_tokens.size() < 3) {
		printf("http::response::from_string(): invalid header line.\n");
		return PARSE_ERROR;
	}

	auto proto = std::tokenize(header_tokens[0], '/');
	if (proto.size() != 2) {
		printf("http::response::from_string(): invalid protocol line \"%s\".\n", header_tokens[2].c_str());
		printf("%s\n", resp.c_str());
		return PARSE_ERROR;
	}

	this->m_proto = proto[0];
	this->m_version = proto[1];

	try {
		this->m_status = std::stoi(header_tokens[1]);
	} catch (std::out_of_range &e) {
		printf("http::response::from_string(): non numeric http status code.\n");
		return PARSE_ERROR;
	}

	this->m_status_str = header_tokens[2];
	for (size_t i = 3; i < header_tokens.size(); i++) {
		this->m_status_str += " " + header_tokens[i];
	}

	// check for content length
	if (this->m_header_fields["content-length"] != "") {
		try {
			int content_length = std::stoi(this->m_header_fields["content-length"]);
			if (content_length > 0 && this->m_body.length() < (size_t)content_length) {
				// printf("http::response::from_string(): response not finished yet %d < %d.\n",
				// (int)this->m_body.length(), content_length);
				return PARSE_ERROR_AGAIN;
			}
		} catch (std::out_of_range &e) {
		}
	}

	// TODO: foreach Set-Cookie
	if (this->m_header_fields["set-cookie"] != "") {
		response_cookie cookie;
		if (cookie.from_string(this->m_header_fields["set-cookie"])) {
			this->m_cookies[cookie.name()] = cookie;
		}
	}

	return PARSE_SUCCESS;
}

std::string
response::to_string(void) {
	this->m_header = this->m_proto + "/" + this->m_version + " " + std::to_string(this->m_status) + " " +
					 status_code_to_str(this->m_status);

	// TODO: multiple cookie support
	for (auto &c : this->m_cookies) {
		this->m_header_fields["set-cookie"] = c.second.to_string();
	}

	return this->header_to_string();
}
