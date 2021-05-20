
#ifndef _WIN32

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/stringutil.h>

#include <networkio/socket/socket.h>
#include <networkio/socket/unix.h>

#include <thread>

using namespace networkio::socket;

//----------------------------------------------------------------------------
// unix_socket
//----------------------------------------------------------------------------

unix_socket::unix_socket() : socket() {}

unix_socket::~unix_socket() {
	this->close();

	// TODO: free send and recv data
}

bool
unix_socket::create_socket() {
	if (this->m_sockfd != INVALID_SOCKET) {
		this->close();
	}

	this->m_sockfd = ::socket(PF_UNIX, SOCK_STREAM, 0);
	if (this->m_sockfd != INVALID_SOCKET) {
		// update blocking state and nopipe if set prior to this
		this->set_blocking(this->m_blocking);
		this->set_nopipe(this->m_nopipe);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
// unix_server
//----------------------------------------------------------------------------

unix_server::unix_server() : unix_socket() {}

unix_server::~unix_server() {
	unix_socket::close();
	::unlink(this->m_path.c_str());
}

bool
unix_server::bind(const u_short port) {
	this->create_socket();
	if (this->m_sockfd == INVALID_SOCKET) {
		return false;
	}

	// unlink path if already exists
	this->m_path = "/tmp/networkio_" + std::to_string(port) + ".sock";
	::unlink(this->m_path.c_str());

	// create address
	struct sockaddr_un serv_addr;
	memset((void *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, this->m_path.c_str(), sizeof(serv_addr.sun_path) - 1);

	// we always want to connect in blocking mode
	bool blocking = this->m_blocking;
	this->set_blocking(true);

	if (::bind(this->m_sockfd, (struct sockaddr *)&serv_addr, SUN_LEN(&serv_addr)) == SOCKET_ERROR)
		return false;

	if (::listen(this->m_sockfd, SOMAXCONN) == SOCKET_ERROR)
		return false;

	this->set_blocking(blocking);

	return true;
}

std::shared_ptr<networkio::interfaces::client>
unix_server::accept(struct sockaddr_in *addr) {
	if (this->m_sockfd == INVALID_SOCKET || this->can_recv() <= 0)
		return nullptr;

	struct sockaddr_in client_addr;
#ifdef _WIN32 // TODO: JUMP TEMPORARY
	int client_addr_size = sizeof(client_addr);
#else
	socklen_t client_addr_size = sizeof(client_addr);
#endif
	SOCKET client_sockfd = ::accept(this->m_sockfd, (struct sockaddr *)&client_addr, &client_addr_size);
	if (client_sockfd == INVALID_SOCKET)
		return nullptr;

	auto socket = std::make_shared<unix_client>();
	socket->m_sockfd = client_sockfd;
	socket->m_connected = true;

	if (!socket->set_blocking(this->m_blocking)) {
		return nullptr;
	}

	if (!socket->set_nopipe(this->m_nopipe)) {
		return nullptr;
	}

	if (addr != nullptr) {
		memcpy(addr, &client_addr, sizeof(struct sockaddr_in));
	}

	return socket;
}

bool
unix_server::close() {
	unix_socket::close();
	::unlink(this->m_path.c_str());
	return true;
}

//----------------------------------------------------------------------------
// unix_client
//----------------------------------------------------------------------------

unix_client::unix_client() : unix_socket() {}

unix_client::~unix_client() {
	this->m_connected = false;
	unix_socket::close();
}

bool
unix_client::connect(const std::string &addr) {
	this->create_socket();
	if (this->m_sockfd == INVALID_SOCKET) {
		return false;
	}

	auto addrtok = std::tokenize(addr, ':');
	if (addrtok.size() != 2) {
		return false;
	}

	std::string path = "/tmp/networkio_" + addrtok[1] + ".sock";

	// construct address
	struct sockaddr_un serv_addr;
	memset((void *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, path.c_str(), sizeof(serv_addr.sun_path) - 1);

	// no non blocking state for unix
	// we always want to connect in blocking mode
	bool blocking = this->m_blocking;
	this->set_blocking(true);

	int error = ::connect(this->m_sockfd, (struct sockaddr *)&serv_addr, SUN_LEN(&serv_addr));
	if (error == SOCKET_ERROR) {
		printf("%s: ::connect() failed: %s\n", __FUNCTION__, strerror(errno));
		this->close();
		return false;
	}

	this->set_blocking(blocking);

	// set connection flag and clear all buffers
	this->m_connected = true;

	return true;
}

bool
unix_client::is_connected() {
	return this->m_connected;
}

bool
unix_client::close() {
	this->m_connected = false;
	return unix_socket::close();
}

int
unix_client::read_raw(uint8_t *buffer, int32_t size) {
	if (!this->is_connected()) {
		return -1;
	}

	if (this->can_recv() <= 0) {
		return 0;
	}

	// unix sockets just read/write entire chunks
	int rcvd = recv(this->m_sockfd, (char *)buffer, size, this->_flags());
	if (rcvd < 0) {
		if (!this->m_blocking) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return 0;
			}
		}
		this->m_connected = false;
		return -1;
	} else if (rcvd == 0) {
		// can_recv was > 0
		return -1;
	}

	return rcvd;
}

int
unix_client::write_raw(const uint8_t *buffer, int32_t size) {
	if (!this->is_connected()) {
		return -1;
	}

	if (this->can_send() <= 0) {
		return 0;
	}

	int sent = send(this->m_sockfd, (char *)buffer, size, this->_flags());
	if (sent < 0) {
		if (!this->m_blocking) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return 0;
			}
		}
		this->m_connected = false;
		return -1;
	} else if (sent == 0) {
		// can_send was > 0
		return -1;
	}
	return sent;
}

#endif
