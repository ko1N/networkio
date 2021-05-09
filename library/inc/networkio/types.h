
#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <Windows.h>
#include <conio.h>
#include <stdarg.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace networkio {
inline void
sleep(unsigned int s) {
	Sleep(s);
}
} // namespace networkio

#define ETHER_ADDR_LEN 6
#define ETHERTYPE_IP 0x0800

#pragma pack(push, 1)
typedef struct ether_header {
	uint8_t ether_dhost[ETHER_ADDR_LEN];
	uint8_t ether_shost[ETHER_ADDR_LEN];
	uint16_t ether_type;
} ETHERHDR;
#pragma pack(pop)

#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH

#elif __APPLE__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/times.h>
#include <sys/un.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

namespace networkio {
inline void
sleep(unsigned int s) {
	usleep(1000 * s);
}
} // namespace networkio

#define DWORD unsigned long
#define PBYTE unsigned char *

inline DWORD
GetTickCount(void) {
	struct timespec sTimeSpec;
	unsigned long ulTick = 0U;
	clock_gettime(CLOCK_REALTIME, &sTimeSpec);
	ulTick = sTimeSpec.tv_nsec / 1000000;
	ulTick += sTimeSpec.tv_sec * 1000;
	return ulTick;
}

#define IPPROTO int

#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define SOCKADDR_IN struct sockaddr_in
#define SOCKADDR struct sockaddr
#define WSAGetLastError() errno

#elif __linux__

#include <errno.h>
#include <memory>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/times.h>
#include <sys/un.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

namespace networkio {
inline void
sleep(unsigned int s) {
	usleep(1000 * s);
}
} // namespace networkio

#define DWORD unsigned long
#define PBYTE unsigned char *

inline DWORD
GetTickCount(void) {
	struct timespec sTimeSpec;
	unsigned long ulTick = 0U;
	clock_gettime(CLOCK_REALTIME, &sTimeSpec);
	ulTick = sTimeSpec.tv_nsec / 1000000;
	ulTick += sTimeSpec.tv_sec * 1000;
	return ulTick;
}

#define IPPROTO int

#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define SOCKADDR_IN struct sockaddr_in
#define SOCKADDR struct sockaddr
#define WSAGetLastError() errno

#elif __unix__

#elif defined(_POSIX_VERSION)

#else

#endif

#include <chrono>

using timepoint_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

#endif
