
#ifndef MEMORY_RING_BUFFER_H_
#define MEMORY_RING_BUFFER_H_

//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include <networkio/types.h>

#include <inttypes.h>
#include <mutex>
#include <string>
#include <vector>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace memory {

//----------------------------------------------------------------------------
// networkio::memory::ring_buffer
//----------------------------------------------------------------------------

class ring_buffer {

public:
  ring_buffer();
  ring_buffer(int32_t initial_size);
  ~ring_buffer();

public:
  void push_back(const uint8_t *buf, int32_t size);
  int32_t pop_front(uint8_t *buf, int32_t size);
  int32_t pop_front(int32_t size);

  template <typename T> T peek() {
    std::lock_guard<std::mutex> lock(this->m_mutex);

    T r{};
    int32_t old_read_offs = this->m_read_offs;
    this->_pop_front((uint8_t *)&r, sizeof(T));
    this->m_read_offs = old_read_offs;

    return r;
  }

  int32_t size();

protected:
  auto _bytes_left() const -> int32_t;
  auto _bytes_written() const -> int32_t;
  void _ensure_capacity(int32_t size);
  int32_t _pop_front(uint8_t *buf, int32_t size);

protected:
  std::mutex m_mutex;
  uint8_t *m_buffer = nullptr;
  int32_t m_buffer_size = 0;
  int32_t m_read_offs = 0;
  int32_t m_write_offs = 0;
  int32_t m_bytes_written = 0;
};

} // namespace memory
} // namespace networkio

#endif
