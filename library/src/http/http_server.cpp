//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <fstream>
#include <list>

#include <networkio/http/http_server.h>

using namespace networkio;
using namespace networkio::http;
using namespace networkio::socket;
using namespace networkio::socket::tcp;

//----------------------------------------------------------------------------
// networkio::http::server_handler
//----------------------------------------------------------------------------

bool server_handler::process() {
  // timeout reached, close client and remove it
  if (!this->check_timeout()) {
    this->m_client->close();
    return false;
  }

  // TODO: this is currently only needed for tls but might change
  // TODO: is this still required? double check!
  // -> this will be later used for string proto handling!
  // -> for now we use raw sockets
  if (!this->m_proto->process()) {
    return false;
  }

  //
  // TODO: this should use recvstring and sendstring (the non blocking variant)
  //
  // read request
  static uint8_t buf[0x4000];
  while (!this->m_request_complete) {
    // read data in if necessary
    int32_t rcvd = this->m_client->read_raw(buf, sizeof(buf));
    if (rcvd < 0) {
      // client called close...
      this->m_client->close();
      return false;
    } else if (rcvd == 0) {
      // nothing done.
      return true;
    }

    // reset timeout and append data
    this->m_request_tp = std::chrono::high_resolution_clock::now();
    this->m_request.append((char *)buf, rcvd);

    // are we done reading yet?
    auto result = this->handle_request();
    switch (result) {
    case PARSE_ERROR: {
      // bogus request
      printf("server_handler::process(): bogus client request.\n");
      this->m_client->close();
      return false;
    }

    case PARSE_ERROR_AGAIN: {
      // not finished, continue loop
      return true;
    } break;

    case PARSE_SUCCESS: {
      // finished
      this->m_request_complete = true;
      this->m_response_buf = (uint8_t *)this->m_response.c_str();
      this->m_response_bytes_left = (int32_t)this->m_response.length();
    } break;
    }
  }

  int32_t sent = this->m_client->write_raw(this->m_response_buf,
                                           this->m_response_bytes_left);
  if (sent < 0) {
    // connection terminated...
    this->m_client->close();
    return false;
  }

  this->m_response_buf += sent;
  this->m_response_bytes_left -= sent;

  if (this->m_response_bytes_left <= 0) {
    // check keep_alive
    if (!this->m_keep_alive) {
      this->m_client->close();
      return false;
    }

    // check maximum number of requests
    this->m_request_count++;
    if (this->m_keep_alive_max > 0 &&
        this->m_request_count >= this->m_keep_alive_max) {
      this->m_client->close();
      return false;
    }

    // reset everything so another request an be processed
    this->m_request = "";
    this->m_request_complete = false;
    this->m_response = "";
  }

  // continue the loop
  return true;
}

bool server_handler::check_timeout() {
  // if keep_alive is disabled dont check for timeout
  if (!this->m_keep_alive) {
    return true;
  }

  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = now - this->m_request_tp;
  if (elapsed.count() > (double)this->m_keep_alive_timeout) {
    return false;
  }

  return true;
}

parser_status server_handler::handle_request() {
  connection conn;

  /*
    hash::hash32_t h;

    try {
            // TODO: create a simple interface for hashing
            h = hash::fnv1a(this->m_request);
            conn.set_request(this->m_request_cache.at(h));
    } catch (std::out_of_range &e) {
            auto res = conn.request().from_string(this->m_request);
            if (res != PARSE_SUCCESS) {
                    // printf("handle_request(): req.from_string() failed.\n");
                    return res;
            }
    }
  */

  auto res = conn.request().from_string(this->m_request);
  if (res != PARSE_SUCCESS) {
    // printf("handle_request(): req.from_string() failed.\n");
    return res;
  }

  auto &req = conn.request();

  // check for keep-alive
  std::string connection = req.header_fields()["connection"];
  std::transform(connection.begin(), connection.end(), connection.begin(),
                 ::tolower);
  this->m_keep_alive = (connection == "keep-alive");
  conn.response().header_fields()["connection"] = connection;
  if (this->m_keep_alive) {
    conn.response().header_fields()["keep-alive"] =
        "timeout=" + std::to_string(this->m_keep_alive_timeout) +
        ", max=" + std::to_string(this->m_keep_alive_max);
  }

  // TODO: this is not thread safe...
  // this->m_request_cache[h] = req;

  // TODO: add request hook here

  // TODO FIXME: add different get and post handler functions
  // TODO: not case sensitive
  if (req.type() == "GET") {
    return this->handle_request_get(conn);
  } else if (req.type() == "POST") {
    return this->handle_request_post(conn);
  } else {
    printf("http::server_handler::handle_request(): invalid request type.\n");
    return PARSE_ERROR;
  }
}

parser_status server_handler::handle_request_get(connection &conn) {
  auto &req = conn.request();
  std::vector<std::string> uri = std::tokenize(req.uri(), '/');

  // super inefficient static file parsing
  //
  // also implement static file caching!!!
  //
  // TODO: i dont like this at all!
  std::shared_ptr<endpoint> e;
  std::string efolder;
  std::string f;
  if (uri.size() > 1) { // TODO: should we check for an asset store on / ?
    std::string u = "";
    for (size_t i = 0; i < uri.size() - 1; i++) {
      u += "/";
      u += uri[i];

      this->m_server->m_endpoints_mutex.lock();
      e = this->m_server->m_endpoints[u];
      if (e != nullptr) {
        efolder = e->m_folder;
      } else {
        efolder = "";
      }
      this->m_server->m_endpoints_mutex.unlock();

      if (!efolder.empty()) {
        // TODO: this is super inefficient
        for (size_t j = i + 1; j < uri.size(); j++) {
          f += "/";
          f += uri[j];
        }

        break;
      } else {
      }
    }
  }

  if (efolder != "") {
    auto &res = conn.response();
    if (res.set_file(efolder + f)) {
      // can we access this file?
      this->m_response = res.to_string();
      return PARSE_SUCCESS;
    }
  }

  // find c++ endpoint in server
  this->m_server->m_endpoints_mutex.lock();
  e = this->m_server->m_endpoints[req.uri()];
  if (e == nullptr || e->m_get == nullptr) {
    e = this->m_server->m_default_endpoint;
  }
  std::function<void(connection &)> ef;
  if (e != nullptr) {
    ef = e->m_get;
  }
  this->m_server->m_endpoints_mutex.unlock();

  // execute endpoint
  if (ef != nullptr) {
    ef(conn);
    this->m_response = conn.response().to_string();
    return PARSE_SUCCESS;
  } else {
    response res(404);
    this->m_response = conn.response().to_string();
    return PARSE_SUCCESS;
  }

  return PARSE_ERROR;
}

parser_status server_handler::handle_request_post(connection &conn) {
  auto &req = conn.request();

  // find endpoint in server
  this->m_server->m_endpoints_mutex.lock();
  auto e = this->m_server->m_endpoints[req.uri()];
  if (e == nullptr || e->m_post == nullptr) {
    e = this->m_server->m_default_endpoint;
  }
  std::function<void(connection &)> ef;
  if (e != nullptr) {
    ef = e->m_post;
  }
  this->m_server->m_endpoints_mutex.unlock();

  if (ef != nullptr) {
    ef(conn);
    this->m_response = conn.response().to_string();
    return PARSE_SUCCESS;
  } else {
    response res(404);
    this->m_response = conn.response().to_string();
    return PARSE_SUCCESS;
  }

  return PARSE_ERROR;
}

//----------------------------------------------------------------------------
// networkio::http::server
//----------------------------------------------------------------------------

server::server() : concurrent_server() {}

server::server(std::shared_ptr<networkio::interfaces::server> sv)
    : concurrent_server(std::move(sv)) {}

server::~server() {}

endpoint_lock server::endpoint_default() {
  if (this->m_default_endpoint == nullptr) {
    this->m_default_endpoint.reset(new class endpoint());
  }

  return endpoint_lock(this->m_default_endpoint, &this->m_endpoints_mutex);
}

endpoint_lock server::endpoint(const std::string &endpoint) {
  if (endpoint.empty()) {
    return endpoint_default();
  }

  std::string _endpoint = endpoint;
  if (_endpoint[0] != '/') {
    _endpoint = std::string("/") + _endpoint;
  }

  if (this->m_endpoints[_endpoint] == nullptr) {
    this->m_endpoints[_endpoint].reset(new class endpoint());
  }

  return endpoint_lock(this->m_endpoints[_endpoint], &this->m_endpoints_mutex);
}

std::shared_ptr<concurrent_server_handler> server::accept_client() {
  struct sockaddr_in clientaddr;
  auto cl = this->m_server->accept(&clientaddr);
  if (cl == nullptr) {
    return nullptr;
  }

  return std::make_shared<class server_handler>(this, std::move(cl));
}

// TODO: check for client timeouts
bool server::process_client(std::shared_ptr<concurrent_server_handler> cl) {
  return cl->process();
}
