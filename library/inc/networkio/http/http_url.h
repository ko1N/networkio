#ifndef HTTP_URL_H_
#define HTTP_URL_H_

//----------------------------------------------------------------------------
// include
//----------------------------------------------------------------------------

#include <string.h>
#include <string>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace http {

/* Converts a hex character to its integer value */
static inline char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + (char)10;
}

/* Converts an integer value to its hex character*/
static inline char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/* Returns a url-encoded version of str */
static inline std::string url_encode(char *str) {
  char *pstr = str, *buf = (char *)malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' ||
        *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> (char)4),
      *pbuf++ = to_hex(*pstr & (char)15);
    pstr++;
  }
  *pbuf = '\0';

  std::string res = buf;
  free(buf);
  return res;
}

/* Returns a url-decoded version of str */
static inline std::string url_decode(char *str) {
  char *pstr = str, *buf = (char *)malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << (char)4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') {
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';

  std::string res = buf;
  free(buf);
  return res;
}

} // namespace http
} // namespace networkio

#endif