
//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#ifndef _WIN32
#include <signal.h>
#endif

#include <ctime>
#include <networkio/networkio.h>
#include <random>
#include <thread>

#include "hostinfo.h"

//----------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------

#define HOST_ADDR "localhost:14447"
const std::string certificate = HOST_CLIENT_CERT;

//----------------------------------------------------------------------------
// Pagesize
//----------------------------------------------------------------------------

uint32_t
getcurrentpagesize(void) {
#if defined(_WIN32) || defined(_WIN64)
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	printf("current page size is %u.\n", si.dwPageSize);
	return (uint32_t)si.dwPageSize;
#else
	printf("current page size is %ld.\n", sysconf(_SC_PAGESIZE));
	return (uint32_t)sysconf(_SC_PAGESIZE);
#endif
}

//----------------------------------------------------------------------------
// Entry Point
//----------------------------------------------------------------------------

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usage: %s -tcp/-udp/-tls/-unix\n", argv[0]);
		return 1;
	}

	uint32_t pagesize = getcurrentpagesize();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, pagesize); // distribute from 1 to pagesize

	// instantiate client
	networkio::mux::client *client = nullptr;

	if (!strcmp(argv[1], "-tcp")) {
		client = new networkio::mux::client();
	} else if (!strcmp(argv[1], "-udp")) {
		auto udpclient = std::make_shared<networkio::socket::udp_client>();
		client = new networkio::mux::client(udpclient);
#ifndef WINDOWS
	} else if (!strcmp(argv[1], "-unix")) {
		auto unixclient = std::make_shared<networkio::socket::unix_client>();
		client = new networkio::mux::client(unixclient);
#endif
#ifdef __WITH_BOTAN__
	} else if (!strcmp(argv[1], "-tls")) {
		auto tlsclient = std::make_shared<networkio::tls::client>();

		tlsclient->set_certificate(certificate);
		tlsclient->set_buffer_size(DEFAULT_PACKET_SIZE);

		// TODO: use smart pointer here
		client = new networkio::mux::client(tlsclient);
#endif
	} else {
		printf("usage: %s -tcp/-udp/-tls/-unix\n", argv[0]);
		return 1;
	}

	client->set_auto_reconnect(5);

	client->event(networkio::mux::event_type::connect, [](networkio::mux::client *client) {
		printf("event: event_type::connect\n");
		networkio::proto::packet hellopkt;
		hellopkt.write_string("hello world.");
		client->call(HASH("hello"), &hellopkt);
	});

	client->event(networkio::mux::event_type::disconnect,
				  [](networkio::mux::client *client) { printf("event: event_type::disconnect\n"); });

	client->event(networkio::mux::event_type::reconnect,
				  [](networkio::mux::client *client) { printf("event: event_type::reconnect\n"); });

	// setup client endpoints
	client->endpoint(HASH("test"), [](networkio::mux::client *client, networkio::proto::packet *packet) {
		static auto start = std::chrono::high_resolution_clock::now();
		static uint64_t rcvd = 0;
		static uint64_t s = 0;

		// count rcvd length
		rcvd += (uint64_t)packet->size();

		// print out stats
		if (s % 5000 == 0) {
			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed = now - start;
			float mbps = (float)(rcvd / 1000000) / elapsed.count();
			printf("received packet #%d with length %d. received %dmb with %2.2f mb/s in %2.2fs.\n", (uint32_t)s,
				   packet->size(), (uint32_t)(rcvd / 1000000), mbps, (float)elapsed.count());
		}

		s++;

		// verify packet
		int c = 0;
		while (packet->read_bytesleft()) {
			uint8_t b = packet->read<uint8_t>();
			if ((uint8_t)(c % 9) != b) {
				printf("failed to verify packet with length 0x%x at position %d.\n", packet->size(), c);
				printf("\n\n\n\n%s\n\n\n\n", packet->to_string().c_str());
				break;
			}
			c++;
		}
	});

	// start insecure server in non blocking mode
	if (!client->connect(HOST_ADDR)) {
		printf("unable to connect to server %s.\n", HOST_ADDR);
		return 0;
	} else {
		printf("connected to server %s.\n", HOST_ADDR);
	}

	while (true) {
		if (client->is_connected()) {
			networkio::proto::packet packet;
			int size = dis(gen);
			packet.ensure_capacity((uint32_t)size);
			for (int i = 0; i < size; i++)
				packet.write((uint8_t)(i % 9));

			client->call(HASH("test"), &packet);
		}

		client->process();
		// networkio::sleep(1);
	}

	return 0;
}
