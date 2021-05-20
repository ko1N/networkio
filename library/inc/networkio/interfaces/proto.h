
#ifndef __INTERFACE_PROTO_H__
#define __INTERFACE_PROTO_H__

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
