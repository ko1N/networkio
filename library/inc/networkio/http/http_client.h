#ifndef HTTP_CLIENT_H_
#define HTTP_CLIENT_H_

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/http/http_request.h>
#include <networkio/http/http_response.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace http {

//----------------------------------------------------------------------------
// networkio::http::client
//----------------------------------------------------------------------------

class client {

public:
  client();
  client(std::shared_ptr<networkio::interfaces::client> cl);
  ~client();

public:
  /*
  TODO:
  - apply custom headers
  - apply custom post data (body)
  */

  double get_timeout() const { return this->m_timeout; }

  void set_timeout(double t) { this->m_timeout = t; }

  int get_follow_redirects() const { return this->m_follow_redirects; }

  void set_follow_redirects(int r) { this->m_follow_redirects = r; }

  networkio::http::response request_sync(std::string url);
  void request_async(
      std::string url,
      std::function<void(const networkio::http::response &)> &&callback);

protected:
  bool build_request(std::string &url, std::string &address,
                     networkio::http::request &req);
  networkio::http::response
  process_request(const std::string &address,
                  const networkio::http::request &request, int redirects = 0);

protected:
  std::shared_ptr<networkio::interfaces::client> m_client;
  double m_timeout = 5.0;
  int m_follow_redirects = 0;
};

} // namespace http
} // namespace networkio

#endif
