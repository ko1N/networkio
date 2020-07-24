#include <mutex>

#include <networkio/posix.h>

namespace networkio {
namespace posix {

std::mutex lock_gethostbyname;

bool
gethostbyname_safe(const std::string &addr, SOCKADDR_IN &out_addr) {
	std::lock_guard<std::mutex> lock(lock_gethostbyname);

	struct hostent *host = ::gethostbyname(addr.c_str());
	if (host == nullptr) {
		return false;
	}

	memcpy(&out_addr.sin_addr, host->h_addr_list[0], host->h_length);
	return true;
}

} // namespace posix
} // namespace networkio