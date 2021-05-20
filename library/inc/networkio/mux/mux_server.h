
#ifndef MUX_SERVER_H_
#define MUX_SERVER_H_

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/fnv1a.h>
#include <networkio/interfaces/server.h>
#include <networkio/proto/packet_proto.h>
#include <networkio/socket/concurrent_server.h>
#include <networkio/socket/socket.h>
#include <networkio/socket/tcp.h>
#include <networkio/stringutil.h>
#include <networkio/types.h>

#include <atomic>
#include <functional>
#include <list>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <networkio/mux/mux_client.h>
#include <networkio/mux/mux_memory.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace mux {

//----------------------------------------------------------------------------
// networkio::mux::server
//----------------------------------------------------------------------------

class server : public networkio::socket::tcp::concurrent_server {

public:
  using callback_t = std::function<void(client *, networkio::proto::packet *)>;

public:
  server();
  server(std::shared_ptr<networkio::interfaces::server> sv);
  ~server();

public:
  // event handlers
  void event(event_type evt, std::function<void(client *)> &&func);

  // simple endpoint communication
  bool endpoint(const hash::hash32_t endpoint, const callback_t &&func);

  // allocates a shared memory region and returns a pointer to it
  template <typename T> T *sharepoint(const hash::hash32_t sharepoint) {
    if (this->m_sharepoints[sharepoint] == nullptr) {
      this->m_sharepoints[sharepoint] = new memory<T>();
    }

    return (T *)this->m_sharepoints[sharepoint]->base();
  }

  // client handlers
  inline void clients_lock() { this->m_client_mutex.lock(); }
  inline void clients_unlock() { this->m_client_mutex.unlock(); }
  std::list<std::shared_ptr<networkio::socket::tcp::concurrent_server_handler>>
      &get_clients() {
    return this->m_clients;
  }

protected:
  virtual std::shared_ptr<networkio::socket::tcp::concurrent_server_handler>
  accept_client() override;
  virtual bool process_client(
      std::shared_ptr<networkio::socket::tcp::concurrent_server_handler> cl)
      override;

protected:
  std::mutex m_endpoints_mutex;
  std::unordered_map<event_type, std::function<void(client *)>, enum_hash>
      m_events;
  std::unordered_map<hash::hash32_t, callback_t> m_endpoints;
  std::unordered_map<hash::hash32_t, base_memory *> m_sharepoints;
};

} // namespace mux
} // namespace networkio

#endif
