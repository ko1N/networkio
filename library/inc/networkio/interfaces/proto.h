
#ifndef INTERFACE_PROTO_H_
#define INTERFACE_PROTO_H_

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
// networkio::interfaces::client
//----------------------------------------------------------------------------

class proto {

public:
  virtual ~proto() {}

  virtual bool process() = 0;
};

} // namespace interfaces
} // namespace networkio

#endif
