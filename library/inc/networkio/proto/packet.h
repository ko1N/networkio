
#ifndef PROTO_PACKET_H_
#define PROTO_PACKET_H_

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/memory/shared_buffer.h>
#include <networkio/types.h>

#include <sstream>
#include <string>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace proto {

//----------------------------------------------------------------------------
// networkio::proto::packet
//----------------------------------------------------------------------------

class packet {

public:
  packet(SOCKADDR_IN *address = nullptr);
  packet(memory::shared_buffer buffer, SOCKADDR_IN *address = nullptr);
  packet(const packet &p);
  ~packet();

public:
  // packet handling
  inline memory::shared_buffer buffer() const { return this->m_buffer; }
  inline uint32_t size() const {
    if (this->m_buffer == nullptr || this->m_write_pointer == nullptr) {
      return 0;
    }
    return (uint32_t)(this->m_write_pointer - this->m_buffer->data());
  }
  inline SOCKADDR_IN *address() { return &this->m_address; }

  // reading
  inline void begin_read() {
    if (this->m_buffer != nullptr) {
      this->m_read_pointer = const_cast<uint8_t *>(this->m_buffer->data());
    }
  }
  inline void set_start() {
    if (this->m_buffer != nullptr && this->m_read_pointer != nullptr) {
      this->m_buffer->set_offset(
          (uint32_t)(this->m_read_pointer - this->m_buffer->data()));
      this->begin_read();
    }
  }
  inline uint8_t *read_buffer() const { return this->m_read_pointer; }
  uint32_t read_bytesleft() const;

  inline bool read_data(uint8_t *buffer, uint32_t length) {
    if (this->read_bytesleft() < length) {
      return false;
    }

    memcpy(buffer, this->m_read_pointer, length);
    this->m_read_pointer += length;
    return true;
  }

  template <typename T> T read() {
    T val;
    if (this->read_data((uint8_t *)&val, sizeof(T))) {
      return val;
    } else {
      return {};
    }
  }

  std::string read_string();

  // writing
  void ensure_capacity(uint32_t length);

  inline void write_data(const uint8_t *buffer, uint32_t length) {
    this->ensure_capacity(length);
    memcpy(this->m_write_pointer, buffer, length);
    this->m_write_pointer += length;
  }

  template <typename T> void write(T val) {
    this->write_data((uint8_t *)&val, sizeof(T));
  }

  void write_string(const std::string &str);

  std::string to_string();

private:
  memory::shared_buffer m_buffer;
  uint8_t *m_read_pointer = nullptr;
  uint8_t *m_write_pointer = nullptr;
  SOCKADDR_IN m_address;
};

} // namespace proto
} // namespace networkio

#endif
