
//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include <list>

#include <networkio/socket/concurrent_server.h>

#define MANY_CONNECTIONS

using namespace networkio::socket::tcp;

//----------------------------------------------------------------------------
// networkio::http::server
//----------------------------------------------------------------------------

// if no argument was supplied we will start a tcp server
concurrent_server::concurrent_server() {
  this->m_server = std::make_shared<networkio::socket::tcp_server>();
}

concurrent_server::concurrent_server(
    std::shared_ptr<networkio::interfaces::server> sv) {
  this->m_server = std::move(sv);
}

concurrent_server::~concurrent_server() { this->shutdown(); }

bool concurrent_server::set_sleep(uint32_t sleep) {
  this->m_sleep = sleep;
  return true;
}

uint32_t concurrent_server::get_sleep() { return this->m_sleep; }

// TODO: realloc thread pool on size changes here
bool concurrent_server::set_threads(uint32_t threads) {
  this->m_threads = threads;
  return true;
}

uint32_t concurrent_server::get_threads() { return this->m_threads; }

bool concurrent_server::run(u_short port) {
  if (!this->start(port)) {
    return false;
  }

  while (this->process()) {
    if (this->m_sleep > 0) {
      networkio::sleep(this->m_sleep);
    }
  }

  return true;
}

bool concurrent_server::start(u_short port) {
  if (this->m_server->bind(port)) {
    this->m_server->set_blocking(false);
    this->m_server->set_nopipe(true);

    // TODO: dynamically adjust threads if setting threads in between calls
    this->m_threads_active = true;
    for (uint32_t i = 0; i < this->m_threads; i++) {
      this->m_threadpool.push_back(
          std::thread(concurrent_server::worker, this));
    }
    return true;
  }

  return false;
}

bool concurrent_server::process() {
  if (this->m_threads > 0) {
    return true;
  }

  // check if new clients try to connect
  auto handler = this->accept_client();
  if (handler != nullptr) {
    // TODO: do not add duplicate clients based on their address...
    this->m_clients.push_back(handler);
  }

  // process existing clients only if multithreading is disabled
  auto iter = this->m_clients.begin();
  while (iter != this->m_clients.end()) {
    if (!this->process_client(*iter)) {
      iter = this->m_clients.erase(iter);
    } else {
      ++iter;
    }
  }

  // return
  return true;
}

void concurrent_server::shutdown() {
  // join threads
  this->m_threads_active = false;
  for (size_t i = 0; i < this->m_threadpool.size(); i++) {
    this->m_threadpool[i].join();
  }
  this->m_threadpool.clear();

  // remove clients
  this->m_client_mutex.lock();
  this->m_clients.clear();
  this->m_client_mutex.unlock();

  // remove socket
  if (this->m_server != nullptr) {
    this->m_server->close();
    this->m_server = nullptr;
  }
}

void concurrent_server::worker(concurrent_server *sv) {

#ifdef MANY_CONNECTIONS

  // TODO: add those clients to client storage
  std::list<std::shared_ptr<concurrent_server_handler>> local_clients;

  if (!sv->m_threads_active) {
    return;
  }

  while (true) {
    // accept
    // TODO: limit amount of parallel sockets
    do {
      auto handler = sv->accept_client();
      if (handler != nullptr) {
        local_clients.push_back(handler);
        // local_clients_queue.push_back(handler);
      }

      // merge
      /*if (local_clients_queue.size() > 0 &&
              sv->m_client_mutex.try_lock()) {
        sv->m_clients.splice(sv->m_clients.begin(), local_clients_queue);
        sv->m_clients.sort();
        sv->m_clients.unique();
        sv->m_client_mutex.unlock();
      }*/

      // process
      /*if (local_clients.size() > 0) {
        auto cl = local_clients.front();
        local_clients.pop_front();

        if (sv->process_client(cl)) {
              local_clients.push_back(cl);
        } else {
              // TODO: remove from sv->m_clients
        }
      }*/
      auto iter = local_clients.begin();
      while (iter != local_clients.end()) {
        if (!sv->process_client(*iter)) {
          iter = local_clients.erase(iter);
        } else {
          ++iter;
        }
      }
    } while (local_clients.size() > 0);

    if (sv->m_sleep > 0) {
      networkio::sleep(sv->m_sleep);
    }
  }

#else

  std::deque<std::shared_ptr<concurrent_server_handler>> local_clients;

  while (true) {
    // fetch new client
    if (local_clients.size() <= 0) {
      // accept new clients since we have nothing todo right now
      auto handler = sv->accept_client();
      if (handler != nullptr) {
        // add to local list since its empty
        local_clients.push_back(handler);
      } else {
        // poll new data if possible or just loop again to accept a client
        if (sv->m_client_mutex.try_lock()) {
          if (sv->m_clients.size() > 0) {
            auto cl = sv->m_clients.front();
            sv->m_clients.pop_front();
            local_clients.push_back(cl);
          }

          sv->m_client_mutex.unlock();
        }
      }
    } else {
      // push new client to backlog
      auto handler = sv->accept_client();

      // we have stuff todo, try to add new handler to the queue
      if (sv->m_client_mutex.try_lock()) {
        // pull oldest client from backlog
        if (sv->m_clients.size() > 0) {
          auto cl = sv->m_clients.front();
          sv->m_clients.pop_front();
          local_clients.push_back(cl);
        }

        if (handler != nullptr) {
          sv->m_clients.push_back(handler);
        }

        sv->m_client_mutex.unlock();
      } else if (handler != nullptr) {
        local_clients.push_back(handler);
      }
    }

    // process client
    if (local_clients.size() > 0) {
      auto cl = local_clients.front();
      local_clients.pop_front();

      if (sv->process_client(cl)) {
        local_clients.push_back(cl);
      }
    }

    // sleep if necessary
    if (sv->m_sleep > 0) {
      networkio::sleep(sv->m_sleep);
    }
  }

#endif
}
