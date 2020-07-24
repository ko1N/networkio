
#ifndef __MUX_MEMORY_H__
#define __MUX_MEMORY_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/socket/socket.h>
#include <networkio/socket/tcp.h>
#include <networkio/stringutil.h>
#include <networkio/types.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace mux {

//----------------------------------------------------------------------------
// networkio::mux::base_memory
//----------------------------------------------------------------------------

class base_memory {

  public:
	virtual ~base_memory() {
		if (this->m_mem != nullptr)
			free(this->m_mem);

		if (this->m_mem_synced != nullptr)
			free(this->m_mem_synced);
	}

  public:
	void *
	base(void) {
		return this->m_mem;
	}

	const void *
	base_synced(void) {
		return this->m_mem_synced;
	}

	void
	sync(void) {
		memcpy(this->m_mem_synced, this->m_mem, this->m_len);
	}

  protected:
	void *m_mem = nullptr;
	void *m_mem_synced = nullptr;
	uint32_t m_len;
};

//----------------------------------------------------------------------------
// networkio::mux::memory
//----------------------------------------------------------------------------

template <class T>
class memory : public base_memory {

  public:
	memory<T>(void) {
		this->m_mem = (T *)malloc(sizeof(T));
		this->m_mem_synced = (T *)malloc(sizeof(T));
		this->m_len = sizeof(T);
	}

  public:
	T *
	base(void) {
		return this->m_mem;
	}
};

} // namespace mux
} // namespace networkio

#endif
