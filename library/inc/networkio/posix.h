
#ifndef __POSIX_H__
#define __POSIX_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/types.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace posix {

bool gethostbyname_safe(const std::string &addr, SOCKADDR_IN &out_addr);

} // namespace posix
} // namespace networkio

#endif
