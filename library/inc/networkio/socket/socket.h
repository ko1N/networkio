
#ifndef __SOCKET_SOCKET_H__
#define __SOCKET_SOCKET_H__

//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include <networkio/types.h>

#if _WIN32

#pragma comment(lib, "wsock32.lib")
#include <stdint.h>

#else

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#endif

#include <inttypes.h>
#include <mutex>
#include <string>
#include <vector>

#include <networkio/interfaces/socket.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace socket {

//----------------------------------------------------------------------------
// defines
//----------------------------------------------------------------------------

#define NET_MAX_MESSAGE (262160 + 0x4000)

//----------------------------------------------------------------------------
// networkio::socket::sockaddr_cmp
//----------------------------------------------------------------------------

int sockaddr_cmp(struct sockaddr *x, struct sockaddr *y);

//----------------------------------------------------------------------------
// networkio::socket::socket
//----------------------------------------------------------------------------

class socket : public virtual interfaces::socket {
  public:
	socket();
	virtual ~socket();

  public:
	SOCKET get_socket();

	int can_recv();
	int can_send();

	virtual bool set_blocking(bool blocking) override;
	virtual bool set_nopipe(bool nopipe) override;

	virtual bool close() override;

  protected:
	int _flags();

  protected:
	SOCKET m_sockfd = INVALID_SOCKET;
	bool m_blocking = true;
	bool m_nopipe = false;
};

} // namespace socket
} // namespace networkio

#endif
