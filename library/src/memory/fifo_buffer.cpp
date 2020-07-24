
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/memory/fifo_buffer.h>

#include <algorithm>
#include <inttypes.h>
#include <mutex>
#include <string>
#include <vector>

using namespace networkio::memory;

//----------------------------------------------------------------------------
// networkio::memory::fifo_buffer
//----------------------------------------------------------------------------

fifo_buffer::fifo_buffer(void) {}

fifo_buffer::~fifo_buffer() {
	if (this->m_buffer != nullptr) {
		free(this->m_buffer);
	}
}

void
fifo_buffer::push_back(const uint8_t *buf, int32_t size) {
	if (size <= 0) {
		return;
	}

	std::lock_guard<std::mutex> lock(this->m_mutex);

	if (this->m_buffer == nullptr) {
		this->m_buffer = (uint8_t *)malloc(size);
		this->m_buffer_size = size;
		memcpy(this->m_buffer, buf, size);
	} else {
		this->m_buffer = (uint8_t *)realloc(this->m_buffer, this->m_buffer_size + size);
		memcpy(this->m_buffer + this->m_buffer_size, buf, size);
		this->m_buffer_size += size;
	}
}

int32_t
fifo_buffer::pop_front(uint8_t *buf, int32_t size) {
	if (size <= 0) {
		return 0;
	}

	std::lock_guard<std::mutex> lock(this->m_mutex);

	int32_t rcvd = std::min(size, this->m_buffer_size);
	if (rcvd <= 0) {
		return 0;
	}

	memcpy(buf, this->m_buffer, rcvd);
	return this->_pop_front(rcvd);
}

int32_t
fifo_buffer::pop_front(int32_t size) {
	if (size <= 0) {
		return 0;
	}

	std::lock_guard<std::mutex> lock(this->m_mutex);
	return this->_pop_front(size);
}

int32_t
fifo_buffer::size(void) {
	std::lock_guard<std::mutex> lock(this->m_mutex);
	return this->m_buffer_size;
}

void
fifo_buffer::clear(void) {
	std::lock_guard<std::mutex> lock(this->m_mutex);
	if (this->m_buffer != nullptr) {
		free(this->m_buffer);
		this->m_buffer_size = 0;
	}
}

int32_t
fifo_buffer::_pop_front(int32_t size) {
	int32_t rcvd = std::min(size, this->m_buffer_size);
	if (rcvd <= 0) {
		return 0;
	}

	if (rcvd == this->m_buffer_size) {
		free(this->m_buffer);
		this->m_buffer = nullptr;
		this->m_buffer_size = 0;
	} else {
		int32_t new_buffer_size = this->m_buffer_size - rcvd;
		uint8_t *new_buffer = (uint8_t *)malloc(new_buffer_size);
		memcpy(new_buffer, this->m_buffer + rcvd, new_buffer_size);
		free(this->m_buffer);
		this->m_buffer = new_buffer;
		this->m_buffer_size = new_buffer_size;
	}

	return rcvd;
}
