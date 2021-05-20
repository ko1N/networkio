
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#define _WINSOCK_DEPRECATED_NO_WARNINGS // TODO: upgrade to new apis
#include <networkio/stringutil.h>

#include <networkio/proto/packet_proto.h>

#include <thread>

using namespace networkio::proto;

//----------------------------------------------------------------------------
// networkio::proto::packet_proto
//----------------------------------------------------------------------------

packet_proto::packet_proto(
    std::shared_ptr<networkio::interfaces::client> client) {
  this->m_client = std::move(client);
  this->m_in_pkt_header.header.magic = 0;
}

packet_proto::~packet_proto() {}

std::shared_ptr<packet> packet_proto::read_packet() {
  if (this->m_client == nullptr || !this->m_client->is_connected()) {
    return nullptr;
  }

  // thread safety
  std::lock_guard<std::mutex> lock(this->m_in_mutex);

  // read in new header if possible
  if (this->m_in_pkt_header.header.magic == 0) {
    while ((uint32_t)this->m_in_buffer.size() > sizeof(pkt_header_t)) {
      this->m_in_pkt_header = this->m_in_buffer.peek<pkt_header_t>();

      if (this->is_valid_pkt_header(&this->m_in_pkt_header)) {
        this->m_in_buffer.pop_front(sizeof(pkt_header_t));
        break;
      }

      printf("%s: invalid header found. skipping byte.\n", __FUNCTION__);
      this->m_in_pkt_header.header.magic = 0; // reset header
      this->m_in_buffer.pop_front(1);
    }
  }

  // valid header found, check if packet is complete yet
  if (this->m_in_pkt_header.header.magic == this->pkt_header_magic &&
      (uint32_t)this->m_in_buffer.size() >=
          this->m_in_pkt_header.header.pkt_size) {
    auto buf =
        std::make_shared<memory::buffer>(this->m_in_pkt_header.header.pkt_size);
    this->m_in_buffer.pop_front(const_cast<uint8_t *>(buf->data()),
                                this->m_in_pkt_header.header.pkt_size);
    this->m_in_pkt_header.header.magic = 0; // reset header
    return std::make_shared<packet>(buf);
  }

  // fetch data
  static uint8_t buf[NET_MAX_MESSAGE];
  static int buf_size = NET_MAX_MESSAGE;

  int32_t read = this->m_client->read_raw(buf, buf_size);
  if (read < 0) {
    // printf("%s: gracefully closed the connection.\n", __FUNCTION__);
    this->m_client->close();
    return nullptr; // gracefully closed the connection
  }

  // do we have to receive the packet as a whole?
  if (!this->m_client->is_stream()) {
    if ((uint32_t)read < sizeof(pkt_header_t)) {
      return nullptr;
    }

    pkt_header_t *hdr = (pkt_header_t *)(buf);
    if (!this->is_valid_pkt_header(hdr) ||
        hdr->header.pkt_size != (uint32_t)read - sizeof(pkt_header_t)) {
      // do not add this packet in the buffer...
      return nullptr;
    }
  }

  this->m_in_buffer.push_back(buf, read);
  return nullptr;
}

bool packet_proto::write_packet(const std::shared_ptr<packet> &p) {
  if (this->m_client == nullptr || !this->m_client->is_connected()) {
    return false;
  }

  this->m_out_mutex.lock();
  this->m_out_pkt_data.push_back(p);
  this->m_out_mutex.unlock();

  return this->process();
}

// TODO: we should use this sendbuffer for raw transmissions as well?!
bool packet_proto::process() {
  if (!this->m_client->is_connected()) {
    return false;
  }

  // thread safety
  std::lock_guard<std::mutex> lock(this->m_out_mutex);

  // nothing to send
  if (this->m_out_pkt_data.empty()) {
    return true;
  }

  // fetch next packet
  // we want to send one consecutive buffer to ensure
  // the packet arrives as a whole when using datagram based protocols
  if (this->m_out_buffer == nullptr) {
    auto p = this->m_out_pkt_data.front();
    this->m_out_buffer = std::make_shared<memory::buffer>(
        (uint32_t)sizeof(pkt_header_t) + p->size());
    pkt_header_t *hdr = (pkt_header_t *)(this->m_out_buffer->data());
    hdr->header.magic = this->pkt_header_magic;
    hdr->header.pkt_size = p->size();
    hdr->hash = hash::fnv1a(&hdr->header, sizeof(hdr->header));
    memcpy(const_cast<uint8_t *>(this->m_out_buffer->data() +
                                 sizeof(pkt_header_t)),
           p->buffer()->data(), p->size());
    this->m_out_data_sent = 0;
  }

  if (this->m_out_buffer != nullptr) {
    int32_t sent = this->m_client->write_raw(
        const_cast<uint8_t *>(this->m_out_buffer->data()) +
            this->m_out_data_sent,
        this->m_out_buffer->size() - this->m_out_data_sent);
    if (sent < 0) {
      // printf("%s: disconnected.\n", __FUNCTION__);
      this->m_client->close();
      return false;
    }

    this->m_out_data_sent += sent;

    if (this->m_out_data_sent == this->m_out_buffer->size()) {
      // finished sending the packet
      this->m_out_pkt_data.pop_front();
      this->m_out_buffer = nullptr;
      this->m_out_data_sent = 0;
    } else if (!this->m_client->is_stream()) {
      // resend this packet entirely on datagram sockets...
      this->m_out_data_sent = 0;
    }
  }

  return true;
}

bool packet_proto::is_valid_pkt_header(const pkt_header_t *hdr) {
  return (hdr->header.magic == this->pkt_header_magic &&
          hash::fnv1a(&hdr->header, sizeof(hdr->header)) == hdr->hash);
}
