//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/http/http_client.h>

using namespace networkio;
using namespace networkio::http;
using namespace networkio::socket;
using namespace networkio::socket::tcp;

//----------------------------------------------------------------------------
// networkio::http::client
//----------------------------------------------------------------------------

client::client() { this->m_client = std::make_shared<networkio::socket::tcp_client>(); }

client::client(std::shared_ptr<networkio::interfaces::client> cl) { this->m_client = std::move(cl); }

client::~client() {}

/*
 * TODO:
 * why not add an error callback class
 * with error description and neat overrides for ifs
 */
/*
TODO: build configuration struct
- follow redirects
*/
networkio::http::response
client::request_sync(std::string url) {
	if (this->m_client == nullptr) {
		return response();
	}

	std::string address;
	networkio::http::request request;
	if (!this->build_request(url, address, request)) {
		printf("http::client::request_sync(): unable to build request.\n");
		return response();
	}

	return this->process_request(address, request, 0);
}

void
client::request_async(std::string url, std::function<void(const networkio::http::response &)> &&callback) {
	std::thread t([&]() {
		auto response = this->request_sync(url);
		callback(response);
	});
	t.detach();
}

bool
client::build_request(std::string &url, std::string &address, networkio::http::request &req) {
	// bool use_tls = true;
	int port = 443;
	std::string hostname = "";
	std::string uri = "/";

	auto proto = std::tokenize(url, "://");
	std::string host = "";
	if (proto.size() == 2) {
		if (proto[0] == "http") {
			// use_tls = false;
			port = 80;
		} else if (proto[0] == "https") {
		}
		host = proto[1];
	} else if (proto.size() > 2) {
		printf("http::client::build_request(): malformed url.\n");
		return false;
	} else {
		host = url;
	}

	auto host_split = host.find_first_of('/');
	if (host_split == std::string::npos) {
		// just acess / on the url, parse port though
		hostname = host;
	} else {
		hostname = host.substr(0, host_split);
	}

	auto port_split = std::tokenize(hostname, ':');
	if (port_split.size() == 2) {
		try {
			hostname = port_split[0];
			port = std::stoi(port_split[1]);
		} catch (std::out_of_range &e) {
			printf("http::client::build_request(): malformed url.\n");
			return false;
		}
	}

	if (host_split != std::string::npos) {
		uri = host.substr(host_split);
	}

	auto uri_split = uri.find_first_of('?');
	if (uri_split != std::string::npos) {
		auto get_params = uri.substr(uri_split + 1);
		uri = uri.substr(0, uri_split);

		// handle get params
		auto params_tok = std::tokenize(get_params, '&');
		for (size_t i = 0; i < params_tok.size(); i++) {
			auto param_tok = std::tokenize(params_tok[i], '=');
			if (param_tok.size() == 2) {
				req.get_params()[param_tok[0]] = param_tok[1];
			}
		}
	}

	req.set_type("get");
	req.set_uri(uri);
	req.header_fields()["host"] = hostname;
	req.header_fields()["user-agent"] =
		"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.5) Gecko/20091102 Firefox/3.5.5 (.NET CLR 3.5.30729)";
	req.header_fields()["connection"] = "close";

	address = hostname + ":" + std::to_string(port);

	// printf("hostname: %s\n", hostname.c_str());
	// printf("uri: %s\n", uri.c_str());
	// printf("address: %s\n", address.c_str());

	return true;
}

networkio::http::response
client::process_request(const std::string &address, const networkio::http::request &request, int redirects) {
	if (this->m_client == nullptr) {
		return response();
	}

	this->m_client->set_blocking(false);
	this->m_client->set_nopipe(true);

	if (!this->m_client->connect(address)) {
		printf("http::client::process_request(): connection failed.\n");
		return response();
	}

	// TODO: string proto
	// transmit request
	std::string header = request.to_string();
	uint8_t *_header = (uint8_t *)header.c_str();
	int32_t _length = (int32_t)header.length();
	while (_length > 0) {
		int32_t sent = this->m_client->write_raw(_header, _length);
		if (sent < 0) {
			printf("http::client::process_request(): unable to send request.\n");
			this->m_client->close();
			return response();
		}
		_header += sent;
		_length -= sent;
	}

	// wait for response
	std::string respstr;
	networkio::http::response resp;
	static uint8_t buf[0x4000];
	clock_t start = clock();
	while (true) {
		// handle timeouts
		if (this->m_timeout > 0.0) {
			double delta = (double)(clock() - start) / CLOCKS_PER_SEC;
			if (delta > this->m_timeout) {
				printf("http::client::process_request(): connection to \"%s\" timed out.\n", address.c_str());
				this->m_client->close();
				break;
			}
		}

		// read until connection is closed
		// TODO: add timeout
		int32_t rcvd = this->m_client->read_raw(buf, sizeof(buf));
		if (rcvd < 0) {
			printf("http::client::process_request(): unable to receive response, retrying #%d times.\n",
				   this->m_follow_redirects - redirects);
			this->m_client->close();
			return this->process_request(address, request, redirects++);
		} else if (rcvd == 0) {
			continue;
		}

		respstr.append((char *)buf, rcvd);

		// finished request + content-length ?
		auto status = resp.from_string(respstr);
		switch (status) {
			case PARSE_ERROR: {
				this->m_client->close();
				return response();
			} break;

			case PARSE_ERROR_AGAIN: {
				// wait for data
			} break;

			case PARSE_SUCCESS: {
				this->m_client->close();

				// TODO: check redirection
				// handle 3xx redirect
				if (resp.status() >= 300 && resp.status() < 400 && redirects < this->m_follow_redirects &&
					resp.header_fields()["location"] != "") {
					// TODO: if this is https connection force redirect to https as well
					std::string redirect_url = resp.header_fields()["location"];
					redirect_url = std::replace_all(redirect_url, "http://", "https://");
					printf("http::client::process_request(): http %d redirect to \"%s\".\n", resp.status(),
						   redirect_url.c_str());

					std::string redirect_address;
					networkio::http::request redirect_request;
					if (!this->build_request(redirect_url, redirect_address, redirect_request)) {
						printf("http::client::process_request(): unable to build redirect request.\n");
						return resp;
					}
					return this->process_request(redirect_address, redirect_request, redirects++);
				}

				return resp;
			} break;
		};
	}

	//
	// TODO: error case will result in empty 404 response :)
	// TODO: connection refused or something
	//
	return response();
}
