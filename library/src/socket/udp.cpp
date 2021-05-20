
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#define _WINSOCK_DEPRECATED_NO_WARNINGS // TODO: upgrade to new apis
#include <networkio/stringutil.h>

#include <networkio/posix.h>
#include <networkio/socket/socket.h>
#include <networkio/socket/udp.h>

#include <algorithm>
#include <thread>

using namespace networkio::socket;

//----------------------------------------------------------------------------
// networkio::socket::udp_socket
//----------------------------------------------------------------------------

udp_socket::udp_socket() : socket() {}

udp_socket::~udp_socket() { this->close(); }

bool
udp_socket::create_socket() {
	if (this->m_sockfd != INVALID_SOCKET)
		return false;

	this->m_sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (this->m_sockfd != INVALID_SOCKET) {
		// update blocking state and nopipe if set prior to this
		this->set_blocking(this->m_blocking);
		this->set_nopipe(this->m_nopipe);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
// networkio::socket::udp_server
//----------------------------------------------------------------------------

udp_server::udp_server() : udp_socket() {}

udp_server::~udp_server() { this->close(); }

bool
udp_server::bind(const u_short port) {
	this->create_socket();
	if (this->m_sockfd == INVALID_SOCKET)
		return false;

	struct sockaddr_in serv_addr;
	memset((void *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // TODO: htonl in socket shit
	serv_addr.sin_port = htons(port);

	// we always want to connect in blocking mode
	bool blocking = this->m_blocking;
	this->set_blocking(true);

	if (::bind(this->m_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		return false;

	this->set_blocking(blocking);

	return true;
}

std::shared_ptr<networkio::interfaces::client>
udp_server::accept(struct sockaddr_in *addr) {
	if (this->m_sockfd == INVALID_SOCKET) {
		return nullptr;
	}

	this->_recv();

	std::lock_guard<std::mutex> lock(this->m_mutex);

	// find new clients and return them here
	if (this->m_new_clients.empty()) {
		return nullptr;
	}

	if (addr != nullptr) {
		memcpy(addr, &this->m_new_clients.front().first, sizeof(struct sockaddr_in));
	}

	auto r = this->m_new_clients.front().second;
	this->m_new_clients.pop_front();
	return r;
}

void
udp_server::_recv() {
	// since we use a static buffer we need to guard lock up here
	std::lock_guard<std::mutex> lock(this->m_mutex);

	if (this->can_recv() <= 0) {
		return;
	}

	// this function is where the magic happens
	// we read the socket and fill out clients or add new clients if needed
	// then new clients are returned or their internal buffer is filled
	static uint8_t buf[NET_MAX_MESSAGE];
	int buf_size = NET_MAX_MESSAGE;
	// memset(buf, 0, buf_size);

	SOCKADDR_IN client_addr;
#ifdef WIN32
	int from_size = sizeof(client_addr);
#else
	socklen_t from_size = sizeof(client_addr);
#endif

	int rcvd = ::recvfrom(this->m_sockfd, (char *)buf, buf_size, 0, (SOCKADDR *)&client_addr, &from_size);
	if (rcvd < 0 || rcvd >= NET_MAX_MESSAGE) {
		if (rcvd < 0 && !this->m_blocking && errno == EAGAIN) {
			return;
		}
		// TODO: disconnect handling
		return;
	}

	// we received a new packet, check our client list
	// if client is in list push the packet to the client
	// if he is not in the list add him to the list and return the client instance
	bool new_client = true;
	for (size_t i = 0; i < this->m_clients.size(); i++) {
		if (!sockaddr_cmp((struct sockaddr *)&std::get<0>(this->m_clients[i]), (struct sockaddr *)&client_addr)) {
			this->m_clients[i].second->_recv(buf, rcvd);
			new_client = false;
		}
	}

	// add new client
	if (new_client) {
		auto cl = std::make_shared<udp_client>(this, this->m_sockfd, &client_addr);
		auto c = std::pair<SOCKADDR_IN, std::shared_ptr<udp_client>>(client_addr, cl);
		this->m_clients.push_back(c);
		this->m_new_clients.push_back(c);
	}
}

//----------------------------------------------------------------------------
// networkio::socket::udp_client
//----------------------------------------------------------------------------

udp_client::udp_client() : udp_socket() {}

udp_client::udp_client(udp_server *server, SOCKET sockfd, SOCKADDR_IN *addr) : udp_socket() {
	this->m_sockfd = sockfd;
	memcpy(&this->m_addr, addr, sizeof(SOCKADDR_IN));
	this->m_connected = true;
	this->m_server = server;
	this->m_should_recv = false;
}

udp_client::~udp_client() {
	// udp client and server uses shared socket so we dont wanna hardclose it
	if (this->m_should_recv) {
		this->m_connected = false;
		socket::close();
	}
}

bool
udp_client::connect(const std::string &addr) {
	this->create_socket();
	if (this->m_sockfd == INVALID_SOCKET) {
		return false;
	}

	auto addrtok = std::tokenize(addr, ':');
	if (addrtok.size() != 2) {
		return false;
	}

	SOCKADDR_IN a;
	a.sin_family = AF_INET;
	a.sin_addr.s_addr = INADDR_ANY;

	if (!networkio::posix::gethostbyname_safe(addrtok[0], a)) {
		return false;
	}

	a.sin_port = htons(atoi(addrtok[1].c_str()));

	// we always want to connect in blocking mode
	bool blocking = this->m_blocking;
	this->set_blocking(true);

	// simulate connect call
	int sent = ::sendto(this->m_sockfd, nullptr, 0, 0, (SOCKADDR *)&a, sizeof(SOCKADDR_IN));
	if (sent < 0) {
		return false;
	}

	this->set_blocking(blocking);

	// set connection flag
	memcpy(&this->m_addr, &a, sizeof(SOCKADDR_IN));
	this->m_connected = true;

	// clear buffers
	this->m_buffer.clear();

	return true;
}

bool
udp_client::is_connected() {
	return this->m_connected;
}

bool
udp_client::close() {
	this->m_connected = false;
	return socket::close();
}

int32_t
udp_client::read_raw(uint8_t *buf, int32_t size) {
	if (this->m_should_recv) {
		if (this->can_recv() <= 0) {
			return 0;
		}

		SOCKADDR_IN server_addr;
#ifdef WIN32
		int from_size = sizeof(server_addr);
#else
		socklen_t from_size = sizeof(server_addr);
#endif

		int rcvd = ::recvfrom(this->m_sockfd, (char *)buf, size, 0, (SOCKADDR *)&server_addr, &from_size);
		if (rcvd < 0 || rcvd >= NET_MAX_MESSAGE) {
			if (rcvd < 0 && !this->m_blocking && errno == EAGAIN) {
				return 0;
			}

			return -1;
		}
		return (int32_t)rcvd;
	} else if (this->m_server != nullptr) {
		this->m_server->_recv();
		return this->m_buffer.pop_front(buf, size);
	}

	return 0;
}

int32_t
udp_client::write_raw(const uint8_t *buf, int32_t size) {
	if (this->can_send() <= 0) {
		return 0;
	}

	int sent = ::sendto(this->m_sockfd, (const char *)buf, size, 0, (SOCKADDR *)&this->m_addr, sizeof(SOCKADDR_IN));
	if (sent < 0 && !this->m_blocking && errno == EAGAIN) {
		return 0;
	}
	return (int32_t)sent;
}

void
udp_client::_recv(const uint8_t *buf, int32_t size) {
	this->m_buffer.push_back(buf, size);
}
