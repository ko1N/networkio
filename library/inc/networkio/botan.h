
#ifndef BOTAN_H_
#define BOTAN_H_

#if defined(_WIN32)
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)
#pragma warning(disable : 4101)

#undef max
#undef min
#endif

#ifdef __WITH_BOTAN__
#include <botan/version.h>
//#include <botan/internal/stl_util.h>

#include <botan/buf_comp.h>
#include <botan/build.h>
#include <botan/parsing.h>
#include <botan/rng.h>

#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
#include <botan/auto_rng.h>
#endif

#if defined(BOTAN_HAS_SYSTEM_RNG)
#include <botan/system_rng.h>
#endif

#include <botan/hex.h>
#include <botan/ocsp.h>
#include <botan/tls_client.h>
#include <botan/tls_handshake_msg.h>
#include <botan/tls_server.h>
#include <botan/x509path.h>

#include <botan/bcrypt.h>
#include <botan/sha160.h>
#include <botan/sha3.h>
#endif

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#endif
