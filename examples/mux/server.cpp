
//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#ifndef _WIN32
#include <signal.h>
#endif

#include <chrono>
#include <networkio/networkio.h>
#include <thread>

#include "hostinfo.h"

//----------------------------------------------------------------------------
// defines
//----------------------------------------------------------------------------

#define HOST_PORT 14447
const std::string certificate = HOST_CLIENT_CERT;
const std::string privatekey = HOST_PRIVATE_KEY;

//----------------------------------------------------------------------------
// structs
//----------------------------------------------------------------------------

using timepoint_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

class user_data : public networkio::memory::base_userdata {
  public:
	timepoint_t start;
	uint64_t rcvd;
	uint64_t s;
};

struct sharepoint_test {
	uint32_t test1;
	uint32_t test2;
	uint8_t test3;
	uint32_t test4;
};

//----------------------------------------------------------------------------
// entry point
//----------------------------------------------------------------------------

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usage: %s -tcp/-udp/-tls/-unix\n", argv[0]);
		return 1;
	}

	// instantiate server
	networkio::mux::server *server = nullptr;

	if (!strcmp(argv[1], "-tcp")) {
		server = new networkio::mux::server();
	} else if (!strcmp(argv[1], "-udp")) {
		auto udpserver = std::make_shared<networkio::socket::udp_server>();
		server = new networkio::mux::server(udpserver);
#ifndef _WIN32
	} else if (!strcmp(argv[1], "-unix")) {
		auto unixserver = std::make_shared<networkio::socket::unix_server>();
		server = new networkio::mux::server(unixserver);
#endif
#ifdef __WITH_BOTAN__
	} else if (!strcmp(argv[1], "-tls")) {
		auto tlsserver = std::make_shared<networkio::tls::server>();

		tlsserver->set_certificate(certificate, privatekey, HOST_PRIVATE_KEY_PASSWORD);
		tlsserver->set_buffer_size(DEFAULT_PACKET_SIZE);

		// TODO: use smart pointer here
		server = new networkio::mux::server(tlsserver);
#endif
	} else {
		printf("usage: %s -tcp/-udp/-tls/-unix\n", argv[0]);
		return 1;
	}

	server->set_sleep(0);
	server->set_threads(2);
	printf("set server threadpool to %d threads.\n", server->get_threads());

	server->event(networkio::mux::event_type::connect,
				  [](networkio::mux::client *client) { printf("event: event_type::connect\n"); });

	server->event(networkio::mux::event_type::disconnect,
				  [](networkio::mux::client *client) { printf("event: event_type::disconnect\n"); });

	server->endpoint(HASH("hello"), [](networkio::mux::client *client, networkio::proto::packet *packet) {
		std::string str = packet->read_string();
		printf("received hello packet: %s\n", str.c_str());
	});

	// setup server endpoints
	server->endpoint(HASH("test"), [](networkio::mux::client *client, networkio::proto::packet *packet) {
		// create a thread safe default userdata object or just return it
		auto d = client->get_userdata<user_data>("", []() {
			user_data *d = new user_data();
			d->start = std::chrono::high_resolution_clock::now();
			d->rcvd = d->s = 0;
			return d;
		});

		// add rcvd length
		d->rcvd += (uint64_t)packet->size();

		// print out stats
		if (d->s % 5000 == 0) {
			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed = now - d->start;
			float mbps = (float)(d->rcvd / 1000000) / elapsed.count();
			printf("received packet #%d with length %d. received %dmb with %2.2f mb/s in %2.2fs.\n", (uint32_t)d->s,
				   packet->size(), (uint32_t)(d->rcvd / 1000000), mbps, (float)elapsed.count());
		}

		d->s++;
		client->call(HASH("test"), packet);
	});

	// sharepoint_test *mem = server->sharepoint<sharepoint_test>("test2");

	// start insecure server in non blocking mode
	while (!server->start(HOST_PORT)) {
		networkio::sleep(5000);
	}

	printf("server started on port %d.\n", HOST_PORT);

	while (true) {
		// server->process();
		// FIXME: this does not work if sleep is removed?!
		networkio::sleep(1000);
	}

	server->shutdown();
	return 0;
}
