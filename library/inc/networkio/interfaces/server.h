
#ifndef __INTERFACE_SERVER_H__
#define __INTERFACE_SERVER_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <memory>
#include <networkio/interfaces/client.h>
#include <networkio/interfaces/socket.h>
#include <networkio/types.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace interfaces {

//----------------------------------------------------------------------------
// networkio::interfaces::server
//----------------------------------------------------------------------------

// this class unifies the server interface for tcp/udp/tls/dtls
class server : public virtual socket {

  public:
	virtual ~server() {}

	virtual bool bind(const u_short port) = 0;
	virtual std::shared_ptr<client> accept(struct sockaddr_in *addr = nullptr) = 0;
};

} // namespace interfaces
} // namespace networkio

#endif
