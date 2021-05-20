
#ifndef __INTERFACE_SOCKET_H__
#define __INTERFACE_SOCKET_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/types.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace interfaces {

//----------------------------------------------------------------------------
// networkio::interfaces::socket
//----------------------------------------------------------------------------

// this class unifies the socket interface for tcp/udp/tls/dtls
class socket {

  public:
	virtual ~socket() {}

	virtual bool is_stream() = 0;

	// TODO: do we really need this as a virtual?
	virtual bool create_socket() = 0;

	virtual bool set_blocking(bool blocking) = 0;
	virtual bool set_nopipe(bool nopipe) = 0;

	virtual bool close() = 0;
};

} // namespace interfaces
} // namespace networkio

#endif
