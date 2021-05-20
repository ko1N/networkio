
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#define _WINSOCK_DEPRECATED_NO_WARNINGS // TODO: upgrade to new apis
#include <networkio/stringutil.h>

#include <networkio/posix.h>
#include <networkio/socket/socket.h>
#include <networkio/socket/tcp.h>

#include <thread>

#ifndef _WIN32
#include <netinet/tcp.h>
#endif

using namespace networkio::socket;

//----------------------------------------------------------------------------
// tcp_socket
//----------------------------------------------------------------------------

tcp_socket::tcp_socket() : socket() {}

tcp_socket::~tcp_socket() {
  this->close();

  // TODO: free send and recv data
}

bool tcp_socket::create_socket() {
  if (this->m_sockfd != INVALID_SOCKET)
    return false;

  this->m_sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (this->m_sockfd != INVALID_SOCKET) {
    // update blocking state and nopipe if set prior to this
#ifndef _WIN32
    // TODO: make this optional
    int flag = 1;
    setsockopt(this->m_sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
#endif
    this->set_blocking(this->m_blocking);
    this->set_nopipe(this->m_nopipe);
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
// tcp_server
//----------------------------------------------------------------------------

tcp_server::tcp_server() : tcp_socket() {}

tcp_server::~tcp_server() { this->close(); }

bool tcp_server::bind(const u_short uPort) {
  this->create_socket();
  if (this->m_sockfd == INVALID_SOCKET)
    return false;

  struct sockaddr_in serv_addr;
  memset((void *)&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // TODO: htonl in socket shit
  serv_addr.sin_port = htons(uPort);

  // we always want to connect in blocking mode
  bool blocking = this->m_blocking;
  this->set_blocking(true);

  if (::bind(this->m_sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) == SOCKET_ERROR)
    return false;

  if (::listen(this->m_sockfd, SOMAXCONN) == SOCKET_ERROR)
    return false;

  this->set_blocking(blocking);

  return true;
}

std::shared_ptr<networkio::interfaces::client>
tcp_server::accept(struct sockaddr_in *addr) {
  if (this->m_sockfd == INVALID_SOCKET || this->can_recv() <= 0)
    return nullptr;

  struct sockaddr_in client_addr;
#ifdef _WIN32 // TODO: JUMP TEMPORARY
  int client_addr_size = sizeof(client_addr);
#else
  socklen_t client_addr_size = sizeof(client_addr);
#endif
  SOCKET client_sockfd = ::accept(
      this->m_sockfd, (struct sockaddr *)&client_addr, &client_addr_size);
  if (client_sockfd == INVALID_SOCKET)
    return nullptr;

  auto sock = std::make_shared<tcp_client>();
  sock->m_sockfd = client_sockfd;
  sock->m_connected = true;

  if (!sock->set_blocking(this->m_blocking) ||
      !sock->set_nopipe(this->m_nopipe)) {
    return nullptr;
  }

  if (addr != nullptr) {
    memcpy(addr, &client_addr, sizeof(struct sockaddr_in));
  }

  return sock;
}

//----------------------------------------------------------------------------
// tcp_client
//----------------------------------------------------------------------------

tcp_client::tcp_client() : tcp_socket() {}

tcp_client::~tcp_client() {
  this->m_connected = false;
  tcp_socket::close();
}

bool tcp_client::connect(const std::string &addr) {
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
    printf("tcp_client::connect(): gethostbyname(\"%s\") failed.\n",
           addrtok[0].c_str());
    return false;
  }

  a.sin_port = htons(std::stoi(addrtok[1]));

  // we always want to connect in blocking mode
  bool blocking = this->m_blocking;
  this->set_blocking(true);

  int error = ::connect(this->m_sockfd, (sockaddr *)&a, sizeof(a));
  if (error == SOCKET_ERROR) {
    // printf("socket() failed: %s\n", strerror(errno));
    return false;
  }

  this->set_blocking(blocking);

  // set connection flag and clear all buffers
  this->m_connected = true;
  return true;
}

bool tcp_client::is_connected() {
  return this->m_sockfd != INVALID_SOCKET && this->m_connected;
}

bool tcp_client::close() {
  this->m_connected = false;
  return tcp_socket::close();
}

bool tcp_client::write_raw_blocked(const uint8_t *pBuffer, int32_t nSize) {
  int32_t nBytesLeft = nSize;
  while (nBytesLeft > 0) {
    if (this->can_send() <= 0)
      continue;

    int sent =
        send(this->m_sockfd, (char *)pBuffer, nBytesLeft, this->_flags());
    if (sent <= 0) {
      this->m_connected = false;
      return false;
    }

    nBytesLeft -= sent;
    pBuffer += sent;
  }

  return true;
}

int32_t tcp_client::read_raw(uint8_t *buf, int32_t len) {
  if (!this->is_connected()) {
    return -1;
  }

  int32_t bytes_left = len;
  while (bytes_left > 0) {
    // we do not need to select on nonblocking sockets
    if (this->can_recv() <= 0) {
      return len - bytes_left;
    }

    int rcvd = ::recv(this->m_sockfd, (char *)buf, bytes_left, this->_flags());
    if (rcvd < 0) {
      if (!this->m_blocking) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          return len - bytes_left;
        }
      }
      this->m_connected = false;
      return -1;
    } else if (rcvd == 0) {
#if defined(LINUX)
      // sometimes errno is set to EAGAIN, this seems to be weird
      if (errno != 0) {
        this->m_connected = false;
        return -1;
      }
      return len - bytes_left;
#elif defined(WINDOWS)
      return len - bytes_left;
#elif defined(OSX)
      return len - bytes_left;
#endif
    }

    bytes_left -= rcvd;
    buf += rcvd;
  }

  // returns how much bytes have been read
  return len - bytes_left;
}

/*
EMSGSIZE
                The socket type requires that message be sent atomically, and
                the size of the message to be sent made this impossible.
*/
int32_t tcp_client::write_raw(const uint8_t *buf, int32_t len) {
  if (!this->is_connected()) {
    return -1;
  }

  int32_t bytes_left = len;
  while (bytes_left > 0) {
    // we do not need to select on nonblocking sockets
    if (this->can_send() <= 0) {
      return len - bytes_left;
    }

    int sent = ::send(this->m_sockfd, (char *)buf, bytes_left, this->_flags());
    if (sent < 0) {
      if (!this->m_blocking) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          return len - bytes_left;
        }
      }
      this->m_connected = false;
      return -1;
    } else if (sent == 0) {
#if defined(LINUX)
      // sometimes errno is set to EAGAIN, this seems to be weird
      if (errno != 0) {
        this->m_connected = false;
        return -1;
      }
      return len - bytes_left;
#elif defined(WINDOWS)
      return len - bytes_left;
#elif defined(OSX)
      return len - bytes_left;
#endif
    }

    bytes_left -= sent;
    buf += sent;
  }

  // returns how much bytes have been sent
  return len - bytes_left;
}
