
#ifndef POSIX_H_
#define POSIX_H_

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/types.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio::posix {

auto gethostbyname_safe(const std::string &addr, SOCKADDR_IN &out_addr) -> bool;

} // namespace networkio::posix

#endif
