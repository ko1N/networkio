
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
	/*
	  networkio::memory::ring_buffer buf(32*1024);
	  const char test[] = "Dies ist ein Test.";
	  //buf.push_back((uint8_t *)test, strlen(test) + 1);
	  //buf.push_back((uint8_t *)test, strlen(test) + 1);
	  buf.push_back((uint8_t *)test, strlen(test) + 1);
	  buf.push_back((uint8_t *)test, strlen(test) + 1);
	  buf.push_back((uint8_t *)test, strlen(test) + 1);
	  buf.push_back((uint8_t *)test, strlen(test) + 1);
	  char test2[1024];
	  buf.pop_front((uint8_t *)test2, strlen(test) + 1);
	  printf("test2: %s\n", test2);
	  buf.push_back((uint8_t *)test, strlen(test) + 1);
	  buf.push_back((uint8_t *)test, strlen(test) + 1);

	  buf.pop_front((uint8_t *)test2, strlen(test) + 1);
	  printf("test2: %s\n", test2);
	  buf.pop_front((uint8_t *)test2, strlen(test) + 1);
	  printf("test2: %s\n", test2);
	  buf.pop_front((uint8_t *)test2, strlen(test) + 1);
	  printf("test2: %s\n", test2);
	  buf.pop_front((uint8_t *)test2, strlen(test) + 1);
	  printf("test2: %s\n", test2);
	  return 0;
	*/
	/*
	  #define BYTES_WRITTEN_TEST 50000000000
	  #define TEST_BUF_SIZE (32*1024)
	  #define TEST_READ_SIZE (512)

	  // test
	  const char str[] = "Test";
	  uint8_t dump[TEST_BUF_SIZE];

	  clock_t start = clock();

	  networkio::memory::ring_buffer buf;

	  std::srand(std::time(nullptr));
	  int64_t _total = 0;
	  while (_total < BYTES_WRITTEN_TEST) {
			buf.push_back((uint8_t *)str, TEST_READ_SIZE);
			buf.pop_front(dump, TEST_READ_SIZE);
			_total += TEST_READ_SIZE;
	  }
	  buf.pop_front(dump, buf.size());
	  buf.push_back((uint8_t *)str, 128);
	  buf.pop_front((uint8_t *)dump, 128);
	  dump[128] = '\0';
	  printf("dump: %s\n", dump);
	  printf("ring_buffer took %f seconds.\n",(float)((double)(clock() - start) /
	  CLOCKS_PER_SEC));

	  start = clock();
	  networkio::memory::fifo_buffer buf2;
	  std::srand(std::time(nullptr));
	  _total = 0;
	  while (_total < BYTES_WRITTEN_TEST) {
			buf2.push_back((uint8_t *)str, TEST_READ_SIZE);
			buf2.pop_front(dump, TEST_READ_SIZE);
			_total += TEST_READ_SIZE;
	  }
	  buf2.pop_front(dump, buf2.size());
	  buf2.push_back((uint8_t *)str, 128);
	  buf2.pop_front((uint8_t *)dump, 128);
	  dump[128] = '\0';
	  printf("dump: %s\n", dump);
	  printf("fifo_buffer took %f seconds.\n",(float)((double)(clock() - start) /
	  CLOCKS_PER_SEC));
	  */

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
