
#ifndef TCP_CONCURRENT_SERVER_H_
#define TCP_CONCURRENT_SERVER_H_

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/interfaces/server.h>
#include <networkio/socket/socket.h>
#include <networkio/socket/tcp.h>
#include <networkio/stringutil.h>
#include <networkio/types.h>

#include <atomic>
#include <functional>
#include <list>
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
namespace socket {
namespace tcp {

//----------------------------------------------------------------------------
// networkio::socket::tcp::concurrent_server_handler
//----------------------------------------------------------------------------

class concurrent_server_handler {

public:
  virtual ~concurrent_server_handler() {}

public:
  virtual bool process() = 0;
};

//----------------------------------------------------------------------------
// networkio::socket::tcp::concurrent_server
//----------------------------------------------------------------------------

class concurrent_server {

public:
  concurrent_server();
  concurrent_server(std::shared_ptr<networkio::interfaces::server> sv);
  virtual ~concurrent_server();

public:
  // configuration
  bool set_sleep(uint32_t sleep);
  uint32_t get_sleep();
  bool set_threads(uint32_t threads);
  uint32_t get_threads();

  // this starts the server in blocking mode and processes all packets
  // internally
  bool run(u_short port);

  // this starts the server in non blocking mode
  bool start(u_short port);

  // processes the server loop if using non blocking mode
  bool process();

  // client accessing

  // shutdown the server and all associated threads
  void shutdown();

protected:
  virtual std::shared_ptr<concurrent_server_handler> accept_client() = 0;
  virtual bool
  process_client(std::shared_ptr<concurrent_server_handler> cl) = 0;
  static void worker(concurrent_server *sv);

protected:
  // 0 sleep will disable sleeping in the process loop
  uint32_t m_sleep = 0;

  // 0 lets it run on main thread, 1 lets it create 1 worker thread
  uint32_t m_threads = 0;

  std::shared_ptr<networkio::interfaces::server> m_server;

  std::mutex m_client_mutex;
  std::list<std::shared_ptr<concurrent_server_handler>> m_clients;

  std::atomic<bool> m_threads_active;
  std::vector<std::thread> m_threadpool;
};

} // namespace tcp
} // namespace socket
} // namespace networkio

#endif
