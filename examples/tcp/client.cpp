
//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#ifndef _WIN32
#include <signal.h>
#endif

#include <networkio/networkio.h>
#include <random>
#include <thread>
#include <time.h>

//----------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------

#define HOST_ADDR "localhost:14447"

//----------------------------------------------------------------------------
// Pagesize
//----------------------------------------------------------------------------

uint32_t
getcurrentpagesize(void) {
#if defined(_WIN32) || defined(_WIN64)
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	printf("current page size is %u.", si.dwPageSize);
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
	uint32_t pagesize = getcurrentpagesize();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, pagesize); // distribute from 1 to pagesize

	auto client = std::make_shared<networkio::socket::tcp_client>();
	auto cl_proto = std::make_shared<networkio::proto::packet_proto>(client);

	client->set_blocking(false);

	if (!client->connect(HOST_ADDR)) {
		printf("unable to connect to server %s.\n", HOST_ADDR);
		return 0;
	} else {
		printf("connected to server %s.\n", HOST_ADDR);
	}

	clock_t start = clock();
	uint64_t rcvd = 0;
	uint64_t s = 0;

	while (true) {
		// generate new packet
		auto outpacket = std::make_shared<networkio::proto::packet>();
		int size = dis(gen);
		outpacket->ensure_capacity((uint32_t)size);
		for (int i = 0; i < size; i++) {
			outpacket->write((uint8_t)(i % 2));
		}

		if (!cl_proto->write_packet(outpacket)) {
			client->close();
			getchar();
			return 0;
		}

		cl_proto->process();

		while (true) {
			auto packet = cl_proto->read_packet();
			if (packet == nullptr) {
				break;
			}

			// count rcvd length
			rcvd += (uint64_t)packet->size();

			// print out stats
			if (s % 5000 == 0) {
				float mbps = (float)(rcvd / 1000000) / ((double)(clock() - start) / CLOCKS_PER_SEC);
				printf(
					"received packet #%d with length %d. received %dmb with %2.2f "
					"mb/s\n",
					(uint32_t)s, packet->size(), (uint32_t)(rcvd / 1000000), mbps);
			}

			// verify packet
			int c = 0;
			while (packet->read_bytesleft()) {
				uint8_t b = packet->read<uint8_t>();
				if ((uint8_t)(c % 2) != b) {
					printf("failed to verify packet with length 0x%x at position %d.\n", packet->size(), c);
					client->close();
					return 0;
				}
				c++;
			}

			s++;
		}

		// Sleep(0);
	}

	return 0;
}
