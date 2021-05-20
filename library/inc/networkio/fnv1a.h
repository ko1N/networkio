#ifndef FNV1A_H_
#define FNV1A_H_

#include <networkio/types.h>

#include <string>

namespace hash {

using hash32_t = uint32_t;

enum MagicNumbers : uint32_t {
  FNV1A_PRIME = 16777619u,
  FNV1A_BIAS = 2166136261u
};

inline hash32_t fnv1a(const void *data, size_t len) {
  if (!data || !len)
    return hash32_t{};

  hash32_t out = FNV1A_BIAS;
  uint8_t *enc = (uint8_t *)data;

  for (size_t i = 0; i < len; ++i) {
    if (enc[i] == '\0') {
      continue;
    }

    out ^= enc[i];
    out *= FNV1A_PRIME;
  }

  return out;
}

inline hash32_t fnv1a_ring(const std::string &str, hash32_t hash = FNV1A_BIAS) {
  hash32_t start = hash;

  for (const auto &c : str) {
    if (c == '\0') {
      continue;
    }

    start ^= uint8_t(c);
    start *= FNV1A_PRIME;
  }

  return start;
}

inline hash32_t fnv1a(const std::string &str) {
  return fnv1a(str.c_str(), str.length());
}

inline hash32_t fnv1a(const std::wstring &str) {
  return fnv1a(str.c_str(), str.length());
}

constexpr hash32_t fnv1a_ct(const char *str, hash32_t start = FNV1A_BIAS) {
  return (*str != '\0') ? fnv1a_ct(str + 1, (*str ^ start) * FNV1A_PRIME)
                        : start;
}

constexpr hash32_t fnv1a_ct(const wchar_t *str, hash32_t start = FNV1A_BIAS) {
  return (*str != '\0') ? fnv1a_ct(str + 1, (*str ^ start) * FNV1A_PRIME)
                        : start;
}

} // namespace hash

#define HASH(str)                                                              \
  ([]() {                                                                      \
    constexpr hash::hash32_t ret = hash::fnv1a_ct(str);                        \
    return ret;                                                                \
  }())

#endif