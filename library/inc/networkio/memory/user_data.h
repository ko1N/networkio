#ifndef MEMORY_USER_DATA_H_
#define MEMORY_USER_DATA_H_

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/types.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace memory {

//----------------------------------------------------------------------------
// networkio::memory::base_userdata
//----------------------------------------------------------------------------

class base_userdata {

public:
  virtual ~base_userdata() {}
};

} // namespace memory
} // namespace networkio

#endif
