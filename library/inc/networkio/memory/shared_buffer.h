
#ifndef __MEMORY_SHARED_BUFFER_H__
#define __MEMORY_SHARED_BUFFER_H__

//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include <networkio/types.h>

#include <inttypes.h>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace memory {

//----------------------------------------------------------------------------
// networkio::memory::shared_buffer
//----------------------------------------------------------------------------

// TODO: add thread safety
class buffer {

  public:
	buffer(uint32_t size);
	buffer(const buffer &o);
	~buffer();

  public:
	const uint8_t *data() const;
	const uint32_t size() const;
	void ensure_capacity(uint32_t size);
	void set_offset(uint32_t offset);
	std::string to_string();

  protected:
	uint8_t *m_buffer = nullptr;
	uint8_t *m_begin = nullptr;
	uint32_t m_offset = 0;
	uint32_t m_size = 0;
};

using shared_buffer = std::shared_ptr<buffer>;

} // namespace memory
} // namespace networkio

#endif
