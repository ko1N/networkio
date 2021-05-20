
#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/fnv1a.h>
#include <networkio/interfaces/server.h>
#include <networkio/proto/string_proto.h>
#include <networkio/socket/concurrent_server.h>
#include <networkio/socket/socket.h>
#include <networkio/socket/tcp.h>
#include <networkio/stringutil.h>
#include <networkio/types.h>

#include <networkio/http/http_request.h>
#include <networkio/http/http_response.h>

#include <atomic>
#include <deque>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace http {

//----------------------------------------------------------------------------
// networkio::http::connection
//----------------------------------------------------------------------------

class connection {

public:
  connection() {}

public:
  inline networkio::http::request &request() { return this->m_request; }

  inline void set_request(const networkio::http::request &r) {
    this->m_request = r;
  }

  inline networkio::http::response &response() { return this->m_response; }

  inline void set_response(const networkio::http::response &r) {
    this->m_response = r;
  }

protected:
  networkio::http::request m_request;
  networkio::http::response m_response;
};

//----------------------------------------------------------------------------
// networkio::http::endpoint
//----------------------------------------------------------------------------

class endpoint {

  friend class server_handler;

public:
  endpoint &get(std::function<void(connection &)> &&func) {
    this->m_get = func;
    return *this;
  }

  std::function<void(connection &)> &get() { return this->m_get; }

  endpoint &post(std::function<void(connection &)> &&func) {
    this->m_post = func;
    return *this;
  }

  std::function<void(connection &)> &post() { return this->m_post; }

  endpoint &folder(const std::string &folder) {
    this->m_folder = folder;
    return *this;
  }

protected:
  std::function<void(connection &)> m_get = nullptr;
  std::function<void(connection &)> m_post = nullptr;
  std::string m_folder = "";
};

//----------------------------------------------------------------------------
// networkio::http::endpoint_lock
//----------------------------------------------------------------------------

class endpoint_lock {

public:
  endpoint_lock(std::shared_ptr<endpoint> ep, std::mutex *ep_mutex) {
    this->m_ep = std::move(ep);
    this->m_ep_mutex = ep_mutex;
    this->m_ep_mutex->lock();
  }

  ~endpoint_lock() { this->m_ep_mutex->unlock(); }

public:
  std::shared_ptr<endpoint> operator->() const { return this->m_ep; }

protected:
  std::shared_ptr<endpoint> m_ep;
  std::mutex *m_ep_mutex;
};

//----------------------------------------------------------------------------
// networkio::http::server_handler
//----------------------------------------------------------------------------

class server;
class server_handler
    : public networkio::socket::tcp::concurrent_server_handler {

  friend class server;

public:
  server_handler(server *sv, const std::shared_ptr<interfaces::client> &cl) {
    this->m_server = sv;
    this->m_client = cl;

    // TODO: implement string proto...
    this->m_proto = std::make_shared<proto::string_proto>(cl);
  }

  ~server_handler() {}

  virtual bool process() override;

protected:
  bool check_timeout();
  parser_status handle_request();
  parser_status handle_request_get(connection &conn);
  parser_status handle_request_post(connection &conn);

protected:
  server *m_server = nullptr;
  std::shared_ptr<interfaces::client> m_client;
  std::shared_ptr<networkio::proto::string_proto> m_proto;

  bool m_keep_alive = false;
  int m_keep_alive_timeout = 5;
  int m_keep_alive_max = 1000;

  int m_request_count = 0;
  std::string m_request = "";
  bool m_request_complete = false;
  timepoint_t m_request_tp;

  std::string m_response = "";
  uint8_t *m_response_buf;
  int32_t m_response_bytes_left;
};

//----------------------------------------------------------------------------
// networkio::http::server
//----------------------------------------------------------------------------

class server : public networkio::socket::tcp::concurrent_server {

  friend class server_handler;

public:
  server();
  server(std::shared_ptr<networkio::interfaces::server> sv);
  ~server();

public:
  endpoint_lock endpoint_default();
  endpoint_lock endpoint(const std::string &endpoint);

protected:
  virtual std::shared_ptr<networkio::socket::tcp::concurrent_server_handler>
  accept_client() override;
  virtual bool process_client(
      std::shared_ptr<networkio::socket::tcp::concurrent_server_handler> cl)
      override;

protected:
  std::mutex m_endpoints_mutex;
  std::shared_ptr<class endpoint> m_default_endpoint;
  std::unordered_map<std::string, std::shared_ptr<class endpoint>> m_endpoints;
};

} // namespace http
} // namespace networkio

#endif
