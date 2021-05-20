
//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#ifndef _WIN32
#include <signal.h>
#endif

#include <fstream>
#include <networkio/networkio.h>
#include <random>
#include <thread>

#include "hostinfo.h"

using namespace networkio;

//----------------------------------------------------------------------------
// defines
//----------------------------------------------------------------------------

#define HOST_ADDR "localhost:14447"
const std::string certificate = HOST_CLIENT_CERT;

header_t header;
std::vector<chunk_t> chunks;

//----------------------------------------------------------------------------
// pagesize
//----------------------------------------------------------------------------

uint32_t
getcurrentpagesize() {
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
// file i/o
//----------------------------------------------------------------------------

inline bool
file_exists(const std::string &name) {
	std::ifstream f(name.c_str());
	return f.good();
}

inline uint64_t
file_size(const std::string &name) {
	std::ifstream f(name.c_str(), std::ifstream::ate | std::ifstream::binary);
	return (uint64_t)f.tellg();
}

inline std::string
file_basename(const std::string &name) {
	return name.substr(name.find_last_of("/\\") + 1);
}

//----------------------------------------------------------------------------
// worker threads
//----------------------------------------------------------------------------

template <typename T>
inline std::shared_ptr<T>
make_shared_array(size_t size) {
	return std::shared_ptr<T>(new T[size], [](T *p) { delete[] p; });
}

class worker {

  public:
	worker() {}
	~worker() {
		this->m_shutdown = true;
		this->m_thread.join();
	}

	bool
	start(const std::string &addr) {
		this->m_client.reset(new socket::udp_client());
		this->m_client->set_blocking(false);

		if (!this->m_client->connect(addr)) {
			return false;
		}

		/*
		this->m_thread = std::thread([this]() {
		  while (!this->m_shutdown) {
				this->m_mutex.lock();
				if (!this->m_queue.empty()) {
				  chunk_t chunk = this->m_queue.front();
				  this->m_queue.pop_front();
				  this->m_mutex.unlock(); // early unlock

				  // construct packet and send it
				  auto packet = std::make_shared<proto::packet>();
				  packet->write<uint32_t>(chunk.count);
				  packet->write<uint32_t>(chunk.size);
				  packet->write<hash::hash32_t>(hash::fnv1a(chunk.buffer.get(),
		chunk.size)); packet->write_data((uint8_t *)chunk.buffer.get(), chunk.size);
				  if (!cl_proto->write_packet(packet)) {
						// TODO: reconnect / retry
				  }
				} else {
				  this->m_mutex.unlock();
				}
				cl_proto->process();
		  }
		});
		*/

		return true;
	}

	void
	add_queue(chunk_t *chunk) {
		this->m_queue.push_back(chunk);
	}

	size_t
	queue_size() {
		return this->m_queue.size();
	}

  protected:
	std::shared_ptr<socket::udp_client> m_client;

	std::atomic_bool m_shutdown{false};
	std::thread m_thread;

	std::list<chunk_t *> m_queue;
};

std::vector<std::shared_ptr<worker>> worker_pool;

void
worker_allocate(std::vector<uint32_t> ports) {
	for (size_t i = 0; i < ports.size(); ++i) {
		auto w = std::make_shared<worker>();
		worker_pool.push_back(w);
	}

	for (size_t i = 0; i < chunks.size(); ++i) {
		auto *c = &chunks[i];
		worker_pool[i % worker_pool.size()]->add_queue(c);
	}

	for (size_t i = 0; i < worker_pool.size(); ++i) {
		printf("worker #%" PRId64 ": %" PRId64 " chunks\n", i, worker_pool[i]->queue_size());
		std::string addr = "localhost:" + std::to_string(ports[i]);
		if (!worker_pool[i]->start(addr)) {
			printf("%s: unable to connect worker to %s.\n", __FUNCTION__, addr.c_str());
		}
	}
}

//----------------------------------------------------------------------------
// entry point
//----------------------------------------------------------------------------

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usage: %s filename\n", argv[0]);
		return 1;
	}

	std::string file_name = argv[1];
	if (!file_exists(file_name)) {
		printf("file %s does not exist.\n", file_name.c_str());
		return 1;
	}

	// TODO: configurable
	header.chunk_size = 32 * getcurrentpagesize();
	header.num_threads = 8;

	header.file_name = file_name;
	// header.file.open(file_name);
	// header.size = file_size(file_name);
	// header.chunks = header.size / header.chunk_size + ((header.size %
	// header.chunk_size) != 0 ? 1 : 0); printf("size: %" PRId64 " mb. chunks: %"
	// PRId64 "\n",
	//  header.size / (1024 * 1024), header.chunks);

	/*  for (uint32_t i = 0; i < header.chunks; i++) {
			chunk_t c;
			c.num = i;
			c.ack = false;
			c.buffer = nullptr;
			chunks.push_back(c);
	  }*/

	// instantiate client
	auto tlsclient = std::make_shared<networkio::socket::tcp_client>();
	// tlsclient->set_certificate(certificate);
	// tlsclient->set_buffer_size(DEFAULT_PACKET_SIZE);

	auto client = std::make_shared<networkio::mux::client>(tlsclient);
	client->set_auto_reconnect(5);

	client->event(networkio::mux::event_type::connect, [&](networkio::mux::client *client) {
		printf("event: event_type::connect\n");
		networkio::proto::packet sendfile;
		sendfile.write<header_t>(header);
		client->call(HASH("sendfile/start"), &sendfile);
	});

	client->endpoint(HASH("sendfile/start"), [&](networkio::mux::client *client, networkio::proto::packet *packet) {
		// connnection accepted, do magic
		uint32_t count = packet->read<uint32_t>();
		printf("sendfile/start %d\n", count);

		std::vector<uint32_t> ports;
		for (uint32_t i = 0; i < count; i++) {
			ports.push_back(packet->read<uint32_t>());
		}

		worker_allocate(ports);
	});

	client->endpoint(HASH("sendfile/chunk_ack"), [&](networkio::mux::client *client, networkio::proto::packet *packet) {
		// acknowledge chunk or decide to resend here if packets have been
		// dropped... fill worker queue with dropped chunks...
	});

	// start insecure server in non blocking mode
	if (!client->connect(HOST_ADDR)) {
		printf("unable to connect to server %s.\n", HOST_ADDR);
		return 0;
	} else {
		printf("connected to server %s.\n", HOST_ADDR);
	}

	while (true) {
		// process
		client->process();

		// TODO: fill queue dynamically

		networkio::sleep(1);
	}

	return 0;
}
