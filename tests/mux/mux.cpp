// test/mux

#include "../tests.h"

#include <networkio/networkio.h>

using namespace networkio;

#define TEST_PORT 23334

BEGIN_TESTS("networkio", "mux")

ADD_TEST("connection test", {
	mux::server server;
	server.set_threads(0);

	mux::client client;

	while (!server.start(TEST_PORT)) {
		networkio::sleep(1000);
	}

	printf("- server opened at port %d\n", TEST_PORT);

	bool connected = false;
	server.endpoint(HASH("test"), [&](mux::client *client, proto::packet *packet) {
		printf("- received test packet\n");
		connected = true;
	});

	client.event(mux::event_type::connect, [](mux::client *client) {
		printf("- sending test packet\n");
		proto::packet pkt;
		pkt.write_string("test");
		TEST(client->call(HASH("test"), &pkt));
	});

	printf("- connecting to server\n");
	TEST(client.connect("127.0.0.1:23334"));

	while (!connected) {
		TEST(client.process());
		TEST(server.process());
		networkio::sleep(1);
	}

	printf("- closing sockets\n");
	client.disconnect();
	server.shutdown();
});

END_TESTS("networkio")
