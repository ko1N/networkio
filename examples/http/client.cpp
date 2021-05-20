
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#ifndef _WIN32
#include <signal.h>
#endif

#include <networkio/networkio.h>
#include <random>
#include <thread>

#include "hostinfo.h"

//----------------------------------------------------------------------------
// entry point
//----------------------------------------------------------------------------
#include <string>
#include <vector>

std::mutex request_mutex;
int requests = 0;

void
worker(const std::string &type, const std::string &url) {
	// TODO: chain validation if cert is not set
	// TODO: auto setup client based on http/https
	std::shared_ptr<networkio::http::client> client;
	if (type == "-tcp") {
		client.reset(new networkio::http::client());
#ifdef __WITH_BOTAN__
	} else if (type == "-tls") {
		auto tlsclient = std::make_shared<networkio::tls::client>();
		client.reset(new networkio::http::client(tlsclient));
#endif
	} else {
		printf("usage: %s -tcp/-tls\n", type.c_str());
		return;
	}

	client->set_follow_redirects(2);

	auto start = std::chrono::high_resolution_clock::now();
	while (true) {
		auto resp = client->request_sync(url);
		// printf("%s\n", resp.to_string().c_str());
		if (resp.status() == 200) {
			request_mutex.lock();
			if (requests % 5000 == 0) {
				auto now = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> elapsed = now - start;
				printf("handled request #%d. handled %2.2f r/q in %2.2fs.\n", requests,
					   (float)((double)requests / elapsed.count()), (float)elapsed.count());
			}
			requests++;
			request_mutex.unlock();
		}
	}
}

int
main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("usage: %s -tcp/-tls url\n", argv[0]);
		return 1;
	}

	printf("spawning 4 worker threads\n");

	std::string type = argv[1];
	std::string url = argv[2];
	for (size_t i = 0; i < 4; i++) {
		std::thread t([=]() { worker(type, url); });
		t.detach();
	}

	while (true) {
		networkio::sleep(1000);
	}

	return 0;
}
