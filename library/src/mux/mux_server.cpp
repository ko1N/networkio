
//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include <functional>
#include <list>
#include <unordered_map>

#include <networkio/mux/mux_server.h>

using namespace networkio::mux;
using namespace networkio::socket;
using namespace networkio::socket::tcp;

//----------------------------------------------------------------------------
// networkio::mux::server
//----------------------------------------------------------------------------

server::server() : concurrent_server() {}

server::server(std::shared_ptr<networkio::interfaces::server> sv)
    : concurrent_server(std::move(sv)) {}

server::~server() {}

void server::event(event_type evt, std::function<void(client *)> &&func) {
  this->m_events[evt] = func;
}

bool server::endpoint(const hash::hash32_t endpoint, const callback_t &&func) {
  // allow endpoint overwriting
  /*if (this->m_endpoints[endpoint] != nullptr) {
                  printf("%s: endpoint is already defined.\n", __FUNCTION__);
                  return false;
  }*/

  this->m_endpoints_mutex.lock();
  this->m_endpoints[endpoint] = func;
  this->m_endpoints_mutex.unlock();

  // printf("%s: registered endpoint %x.\n", __FUNCTION__, endpoint);
  return true;
}

std::shared_ptr<concurrent_server_handler> server::accept_client() {
  struct sockaddr_in clientaddr;
  auto cl = this->m_server->accept(&clientaddr);
  if (cl == nullptr) {
    return nullptr;
  }

  auto client = std::make_shared<class client>();
  client->m_client = cl;
  client->m_proto = std::make_shared<networkio::proto::packet_proto>(cl);
  client->m_connection_state = 1;

  // register disconnect client event to relay to the server
  client->event(event_type::disconnect, [&](networkio::mux::client *cl) {
    auto ev = this->m_events[event_type::disconnect];
    if (ev != nullptr) {
      ev(cl);
    }
  });

  auto ev = this->m_events[event_type::connect];
  if (ev != nullptr) {
    // TODO: should we use smart pointers here?
    ev((class client *)client.get());
  }

  // TODO: register a client disconnect handler here so we can remove it from
  // the list?

  return client;
}

// TODO: check for client timeouts
bool server::process_client(std::shared_ptr<concurrent_server_handler> _cl) {
  // FIXME i know this is not neat design
  client *cl = (client *)_cl.get();

  if (cl->m_proto == nullptr) {
    printf("%s: proto is not setup properly\n", __FUNCTION__);
    return false;
  }

  if (!cl->m_proto->process()) {
    auto ev = this->m_events[event_type::disconnect];
    if (ev != nullptr) {
      ev(cl);
    }
    return false;
  }

  // TODO: limit packet amount per client in one go
  while (true) {
    // do we have a new packet?
    auto p = cl->m_proto->read_packet();
    if (p == nullptr) {
      break;
    }

    // read endpoint
    hash::hash32_t endpoint = p->read<hash::hash32_t>();
    if (endpoint != MUX_HEARTBEAT && p->read_bytesleft() > 0) {
      this->m_endpoints_mutex.lock();

      if (this->m_endpoints.count(endpoint) > 0 &&
          this->m_endpoints[endpoint] != nullptr) {
        p->set_start();
        this->m_endpoints[endpoint](cl, p.get());
      } else {
        printf("%s: missing endpoint \"%d\"\n", __FUNCTION__, endpoint);
      }

      this->m_endpoints_mutex.unlock();
    }
  }

  // check for timeout and send a heartbeat
  if (!cl->check_heartbeat()) {
    return false;
  }

  return true;
}
