
#ifndef MEMORY_FIFO_BUFFER_H_
#define MEMORY_FIFO_BUFFER_H_

//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include <networkio/types.h>

#include <cinttypes>
#include <mutex>
#include <string>
#include <vector>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace memory {

//----------------------------------------------------------------------------
// networkio::memory::fifo_buffer
//----------------------------------------------------------------------------

class fifo_buffer {

public:
  fifo_buffer();
  ~fifo_buffer();

  void push_back(const uint8_t *buf, int32_t size);
  int32_t pop_front(uint8_t *buf, int32_t size);
  int32_t pop_front(int32_t size);

  template <typename T> T peek() {
    std::lock_guard<std::mutex> lock(this->m_mutex);
    T r{};
    if (this->m_buffer != nullptr) {
      r = *(T *)this->m_buffer;
    }
    return r;
  }

  auto size() -> int32_t;
  void clear();

protected:
  int32_t _pop_front(int32_t size);

  std::mutex m_mutex;
  uint8_t *m_buffer = nullptr;
  int32_t m_buffer_size = 0;
};

} // namespace memory
} // namespace networkio

#endif
