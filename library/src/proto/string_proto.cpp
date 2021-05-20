
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#define _WINSOCK_DEPRECATED_NO_WARNINGS // TODO: upgrade to new apis

#include <thread>

#include <networkio/proto/string_proto.h>
#include <networkio/stringutil.h>

using namespace networkio::proto;

//----------------------------------------------------------------------------
// networkio::proto::string_proto
//----------------------------------------------------------------------------

string_proto::string_proto(
    std::shared_ptr<networkio::interfaces::client> client) {
  this->m_client = std::move(client);
}

string_proto::~string_proto() {}

std::string string_proto::read_string() {
  if (this->m_client == nullptr || !this->m_client->is_connected()) {
    return "";
  }

  return "";

  /*std::string sResult;
  char cBuffer[4096];
  memset(cBuffer, 0, sizeof(cBuffer));
  int rcvd = recv(this->m_sockfd, cBuffer, sizeof(cBuffer) - 1, this->_flags());
  if (rcvd < 0)
                  return ""; // TODO: check if the connection has been closed?

  sResult.append(cBuffer, rcvd);
  return sResult;*/
}

bool string_proto::write_string(const std::string &s) {
  if (this->m_client == nullptr || !this->m_client->is_connected())
    return false;

  //	this->m_out_string += sString;

  // return this->process_string();
  return true;
}

// TODO: we should use this sendbuffer for raw transmissions as well?!
bool string_proto::process() {
  if (this->m_out_string == "") {
    return true;
  }

  return true;
  /*
                  int sent = this->write_raw((uint8_t
     *)this->m_out_string.c_str(), (uint32_t)this->string_proto.size()); if
     (sent < 0) { this->m_client->close(); return -1; // disconnected
                  }

                  if (sent > 0) {
                                  if ((size_t)sent == this->m_out_string.size())
     { this->m_out_string = ""; return 0; } else { this->m_out_string =
     this->m_out_string.substr((size_t)sent); return
     (uint32_t)this->m_out_string.size();
                                  }
                  }

                  return (uint32_t)this->m_out_string.size();
    */
}
