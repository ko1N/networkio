
#ifndef PROTO_PACKET_PROTO_H_
#define PROTO_PACKET_PROTO_H_

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/fnv1a.h>
#include <networkio/interfaces/client.h>
#include <networkio/interfaces/proto.h>
#include <networkio/memory/fifo_buffer.h>
#include <networkio/proto/packet.h>
#include <networkio/socket/socket.h>
#include <networkio/types.h>

#include <deque>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace proto {

//----------------------------------------------------------------------------
// networkio::proto::packet_proto
//----------------------------------------------------------------------------

// TODO: create another interface for packet protocols...

class packet_proto : public virtual interfaces::proto {

public:
  static const uint8_t pkt_header_magic = 0xFF;
  struct pkt_header_t {
    struct {
      uint32_t magic;
      uint32_t pkt_size;
    } header;
    hash::hash32_t hash;
  };

public:
  packet_proto(std::shared_ptr<networkio::interfaces::client> client);
  virtual ~packet_proto();

public:
  std::shared_ptr<packet> read_packet();
  bool write_packet(const std::shared_ptr<packet> &p);

  virtual bool process() override;

protected:
  bool is_valid_pkt_header(const pkt_header_t *hdr);

protected:
  std::shared_ptr<networkio::interfaces::client> m_client;

  // recv
  std::mutex m_in_mutex;
  pkt_header_t m_in_pkt_header;
  networkio::memory::fifo_buffer m_in_buffer;

  // send
  std::mutex m_out_mutex;
  std::deque<std::shared_ptr<packet>> m_out_pkt_data;
  networkio::memory::shared_buffer m_out_buffer;
  uint32_t m_out_data_sent = 0;
};

} // namespace proto
} // namespace networkio

#endif
