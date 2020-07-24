
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#ifndef _WIN32
#include <signal.h>
#endif

#include <ctime>
#include <iomanip>
#include <iostream>
#include <networkio/networkio.h>
#include <sstream>
#include <thread>

#include "hostinfo.h"

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

#define HOST_PORT 14447
const std::string certificate = HOST_CLIENT_CERT;
const std::string privatekey = HOST_PRIVATE_KEY;

//----------------------------------------------------------------------------
// http_session_data
//----------------------------------------------------------------------------

class http_session_data : public networkio::memory::base_userdata {
  public:
	bool logged_in = false;
};

//----------------------------------------------------------------------------
// entry point
//----------------------------------------------------------------------------

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usage: %s -tcp/-tls\n", argv[0]);
		return 1;
	}

	// instantiate server
	networkio::http::server *server = nullptr;
	if (!strcmp(argv[1], "-tcp")) {
		server = new networkio::http::server();
#ifdef __WITH_BOTAN__
	} else if (!strcmp(argv[1], "-tls")) {
		auto tlsserver = std::make_shared<networkio::tls::server>();

		tlsserver->set_certificate(certificate, privatekey, HOST_PRIVATE_KEY_PASSWORD);
		tlsserver->set_buffer_size(DEFAULT_PACKET_SIZE);

		// TODO: use smart pointer here
		server = new networkio::http::server(tlsserver);
#endif
	} else {
		printf("usage: %s -tcp/-tls\n", argv[0]);
		return 1;
	}

	server->set_sleep(1);
	server->set_threads(2);
	printf("set server threadpool to %d threads.\n", server->get_threads());

	// todo either global or in http server
	static networkio::http::session_store store;

	server->endpoint("/")
		->get([](networkio::http::connection &conn) {
			auto session = store.get(conn);
			auto data = session->get_userdata<http_session_data>("");

			// check for valid login
			auto params = conn.request().get_params();
			if (params["username"] == "admin" && params["password"] == "test") {
				data->logged_in = true;
			}

			std::string r =
				"\
			<!DOCTYPE html>\
			<html>\
			<body>";
			if (data->logged_in) {
				// display logout button
				r += "\
				<h2>Login</h2>\
				<form action='/logout' method='get'>\
					<button type='submit'>Logout</button>\
				</form>";
			} else {
				// display login form
				r += "\
				<h2>Login</h2>\
				<form method='post'>\
					<input type='text' name='username' placeholder='username' />\
					<input type='password' name='password' placeholder='password' />\
					<button type='submit'>Login</button>\
				</form>";
			}
			r += "\
			</div>\
			</body>\
			</html>";

			conn.response().set_body(r);
		})
		.post([=](networkio::http::connection &conn) {
			// pass this to the get request handler
			server->endpoint("/")->get()(conn);
		});

	server->endpoint("/logout")->get([=](networkio::http::connection &conn) {
		auto session = store.get(conn);
		auto data = session->get_userdata<http_session_data>("");
		data->logged_in = false;

		// redirect to /
		conn.response().set_status(302);
		conn.response().header_fields()["location"] = "/";
	});

	server->endpoint_default()->get([](networkio::http::connection &conn) {
		printf("unknown request: %s\n", conn.request().uri().c_str());
		conn.response().set_status(404);
	});

	while (!server->start(HOST_PORT)) {
		networkio::sleep(5000);
	}

	printf("server started on port %d.\n", HOST_PORT);

	while (true) {
		// server->process(); // TODO: if started threaded ignore process() calls
		// FIXME: this does not work if sleep is removed?!
		networkio::sleep(1000);
	}

	server->shutdown();
	return 0;
}
