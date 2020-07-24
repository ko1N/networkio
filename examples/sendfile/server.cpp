
//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#ifndef _WIN32
#include <signal.h>
#endif

#include <inttypes.h>
#include <networkio/networkio.h>
#include <thread>

#include "hostinfo.h"

using namespace networkio;

//----------------------------------------------------------------------------
// defines
//----------------------------------------------------------------------------

#define HOST_PORT 14447
const std::string certificate = HOST_CLIENT_CERT;
const std::string privatekey = HOST_PRIVATE_KEY;

header_t header;
std::list<chunk_t> chunks;

//----------------------------------------------------------------------------
// worker threads
//----------------------------------------------------------------------------

class worker {

  public:
	worker(void) {}
	~worker() {
		this->m_shutdown = true;
		this->m_thread.join();
	}

	bool
	start(u_short port) {
		this->m_server.reset(new socket::udp_server());
		this->m_server->set_blocking(false);

		if (!this->m_server->bind(port)) {
			return false;
		}

		this->m_port = port;
		this->m_thread = std::thread([this]() {
			while (!this->m_shutdown) {
				struct sockaddr_in clientaddr;
				auto client = this->m_server->accept(&clientaddr); // TODO: create shared pointer here
				if (client == nullptr) {
					continue;
				}

				// TODO: handle thread safety?
				while (true) {
					// client->read_raw();

					/*
					//printf("%s: received packet.\n", __FUNCTION__);
					uint32_t chunk = packet->read<uint32_t>();
					uint32_t size = packet->read<uint32_t>();
					hash::hash32_t hash = packet->read<hash::hash32_t>();
					// data...
					if ((chunk % 1000) == 0) {
					  printf("%s: received chunk %d.\n",
							__FUNCTION__, chunk);
					}
					*/
				}
			}
		});
		this->m_thread.detach();

		return true;
	}

	uint32_t
	get_port(void) {
		return this->m_port;
	}

  protected:
	std::shared_ptr<socket::udp_server> m_server;
	uint32_t m_port = 0;

	std::atomic_bool m_shutdown{false};
	std::thread m_thread;

	std::mutex m_mutex;
	// deque buffer something
};

std::vector<std::shared_ptr<worker>> worker_pool;

void
worker_allocate(int32_t count) {
	for (int32_t i = 0; i < count; i++) {
		auto w = std::make_shared<worker>();
		if (!w->start(HOST_PORT + 1 + i)) {
			printf("%s: unable to start worker on port %d.\n", __FUNCTION__, HOST_PORT + i);
			continue;
		}

		worker_pool.push_back(w);
	}
}

//----------------------------------------------------------------------------
// entry point
//----------------------------------------------------------------------------

int
main(int argc, char *argv[]) {
	// instantiate server
	auto tlsserver = std::make_shared<socket::tcp_server>();
	// tlsserver->set_certificate(certificate, privatekey, HOST_PRIVATE_KEY_PASSWORD);
	// tlsserver->set_buffer_size(DEFAULT_PACKET_SIZE);

	mux::server *server = new mux::server(tlsserver);
	server->set_sleep(0);
	server->set_threads(0);

	server->endpoint(HASH("sendfile/start"), [](mux::client *client, proto::packet *packet) {
		header = packet->read<header_t>();
		printf("sendfile/start %d\n", header.num_threads);

		// spawn worker threads here and send all ports to the
		// client
		worker_allocate(header.num_threads);

		proto::packet res;
		res.write<uint32_t>((uint32_t)worker_pool.size());
		for (auto &w : worker_pool) {
			res.write<uint32_t>(w->get_port());
		}
		client->call(HASH("sendfile/start"), &res);
	});

	// start insecure server in non blocking mode
	while (!server->start(HOST_PORT)) {
		networkio::sleep(5000);
	}

	printf("server started on port %d.\n", HOST_PORT);

	while (true) {
		server->process();
		networkio::sleep(1);
	}

	server->shutdown();
	return 0;
}
