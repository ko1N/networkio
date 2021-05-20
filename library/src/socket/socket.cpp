
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#define _WINSOCK_DEPRECATED_NO_WARNINGS // TODO: upgrade to new apis
#include <networkio/stringutil.h>

#include <networkio/socket/socket.h>

using namespace networkio::socket;

//----------------------------------------------------------------------------
// networkio::socket::sockaddr_cmp
//----------------------------------------------------------------------------

int networkio::socket::sockaddr_cmp(struct sockaddr *x, struct sockaddr *y) {
#define CMP(a, b)                                                              \
  if ((a) != (b))                                                              \
  return (a) < (b) ? -1 : 1

  CMP(x->sa_family, y->sa_family);

#ifndef _WIN32
  if (x->sa_family == AF_UNIX) {
    struct sockaddr_un *xun = (struct sockaddr_un *)x,
                       *yun = (struct sockaddr_un *)y;
    int r = strcmp(xun->sun_path, yun->sun_path);
    if (r != 0)
      return r;
  } else
#endif
      if (x->sa_family == AF_INET) {
    struct sockaddr_in *xin = (struct sockaddr_in *)x,
                       *yin = (struct sockaddr_in *)y;
    CMP(ntohl(xin->sin_addr.s_addr), ntohl(yin->sin_addr.s_addr));
    CMP(ntohs(xin->sin_port), ntohs(yin->sin_port));
  } else if (x->sa_family == AF_INET6) {
    struct sockaddr_in6 *xin6 = (struct sockaddr_in6 *)x,
                        *yin6 = (struct sockaddr_in6 *)y;
    int r = memcmp(xin6->sin6_addr.s6_addr, yin6->sin6_addr.s6_addr,
                   sizeof(xin6->sin6_addr.s6_addr));
    if (r != 0)
      return r;
    CMP(ntohs(xin6->sin6_port), ntohs(yin6->sin6_port));
    CMP(xin6->sin6_flowinfo, yin6->sin6_flowinfo);
    CMP(xin6->sin6_scope_id, yin6->sin6_scope_id);
  } else {
    return -1;
  }

#undef CMP
  return 0;
}

//----------------------------------------------------------------------------
// networkio::socket::socket
//----------------------------------------------------------------------------

socket::socket() {
#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif
}

socket::~socket() {}

SOCKET
socket::get_socket() { return this->m_sockfd; }

int socket::can_recv() {
  if (this->m_sockfd == INVALID_SOCKET)
    return -1;

  fd_set sockread;
  struct timeval wait = {0, 0};

  FD_ZERO(&sockread);
  FD_SET(this->m_sockfd, &sockread);

  if (select(FD_SETSIZE, &sockread, 0, 0, &wait) < 0) {
    return -1;
  }

  return FD_ISSET(this->m_sockfd, &sockread);
}

int socket::can_send() {
  if (this->m_sockfd == INVALID_SOCKET)
    return -1;

  fd_set sockwrite;
  struct timeval wait = {0, 0};

  FD_ZERO(&sockwrite);
  FD_SET(this->m_sockfd, &sockwrite);

  if (select(FD_SETSIZE, 0, &sockwrite, 0, &wait) < 0) {
    return -1;
  }

  return FD_ISSET(this->m_sockfd, &sockwrite);
}

bool socket::set_blocking(bool blocking) {
  if (this->m_sockfd == INVALID_SOCKET) {
    this->m_blocking = blocking;
    return true;
  }

#ifdef _WIN32
  DWORD nonblocking = blocking ? 0 : 1;
  if (ioctlsocket(this->m_sockfd, FIONBIO, &nonblocking) >= 0) {
    this->m_blocking = blocking;
    return true;
  }
#else
  int fl = fcntl(this->m_sockfd, F_GETFL, 0);
  if (fcntl(this->m_sockfd, F_SETFL,
            (blocking ? (fl & ~O_NONBLOCK) : (fl | O_NONBLOCK))) >= 0) {
    this->m_blocking = blocking;
    return true;
  }
#endif

  return false;
}

bool socket::set_nopipe(bool nopipe) {
  if (this->m_sockfd == INVALID_SOCKET) {
    this->m_nopipe = nopipe;
    return true;
  }

#if !defined(_WIN32) && !defined(__linux__)
  int p = nopipe ? 1 : 0;
  if (setsockopt(this->m_sockfd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&p,
                 sizeof(p)) < 0) {
    return false;
  }
#endif

  this->m_nopipe = nopipe;

  return true;
}

bool socket::close() {
  if (this->m_sockfd != INVALID_SOCKET) {
    ::shutdown(this->m_sockfd, SHUT_RDWR);
#ifdef _WIN32
    ::closesocket(this->m_sockfd);
#else
    ::close(this->m_sockfd);
#endif
    this->m_sockfd = INVALID_SOCKET;
    return true;
  }

  return false;
}

int socket::_flags() {
#ifdef __linux__
  if (this->m_nopipe) {
    return MSG_NOSIGNAL;
  } else {
    return 0;
  }
#else
  return 0;
#endif
}
