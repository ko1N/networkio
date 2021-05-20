
#ifndef INTERFACE_CLIENT_H_
#define INTERFACE_CLIENT_H_

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/interfaces/socket.h>
#include <networkio/types.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace interfaces {

//----------------------------------------------------------------------------
// networkio::interfaces::client
//----------------------------------------------------------------------------

// this class unifies the client interface for tcp/udp/tls/dtls
class client : public virtual socket {

public:
  virtual ~client() {}

  virtual bool connect(const std::string &addr) = 0;
  virtual bool is_connected() = 0;

  virtual int32_t read_raw(uint8_t *buf, int32_t len) = 0;
  virtual int32_t write_raw(const uint8_t *buf, int32_t len) = 0;
};

} // namespace interfaces
} // namespace networkio

#endif
