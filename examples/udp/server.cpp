
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#ifndef _WIN32
#include <signal.h>
#endif

#include <networkio/networkio.h>
#include <thread>
#include <time.h>

//----------------------------------------------------------------------------
// defines
//----------------------------------------------------------------------------

#define HOST_PORT 14447

//----------------------------------------------------------------------------
// client thread
//----------------------------------------------------------------------------

void
client_thread(const std::shared_ptr<networkio::interfaces::client> &client) {
	clock_t start = clock();
	uint64_t rcvd = 0;
	uint64_t s = 0;

	auto cl_proto = std::make_shared<networkio::proto::packet_proto>(client);

	while (true) {
		cl_proto->process();

		while (true) {
			auto packet = cl_proto->read_packet();
			if (packet == nullptr) {
				break;
			}

			// add rcvd length
			rcvd += (uint64_t)packet->size();

			// print out stats
			if (s % 5000 == 0) {
				float mbps = (float)(rcvd / 1000000) / ((double)(clock() - start) / CLOCKS_PER_SEC);
				printf(
					"received packet #%d with length %d. received %dmb with %2.2f "
					"mb/s\n",
					(uint32_t)s, packet->size(), (uint32_t)(rcvd / 1000000), mbps);
			}

			cl_proto->write_packet(packet);
			s++;
		}
	}
}

//----------------------------------------------------------------------------
// entry point
//----------------------------------------------------------------------------

int
main(int argc, char *argv[]) {
	networkio::socket::udp_server server;

	// set socket to non blocking
	server.set_blocking(false);

	if (!server.bind(HOST_PORT)) {
		printf("unable to bind server socket to port %d.\n", HOST_PORT);
		return 0;
	} else {
		printf("server socket bound to port %d.\n", HOST_PORT);
	}

	while (true) {
		struct sockaddr_in clientaddr;
		auto client = server.accept(&clientaddr);
		if (client != nullptr) {
			std::thread thread(client_thread, client);
			thread.detach();
		}
	}

	return 0;
}
