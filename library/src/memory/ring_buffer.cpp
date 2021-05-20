
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/memory/ring_buffer.h>

#include <algorithm>
#include <inttypes.h>
#include <mutex>
#include <string>
#include <vector>

using namespace networkio::memory;

//----------------------------------------------------------------------------
// networkio::memory::ring_buffer
//----------------------------------------------------------------------------

ring_buffer::ring_buffer() {}

ring_buffer::ring_buffer(int32_t initial_size) { this->_ensure_capacity(initial_size); }

ring_buffer::~ring_buffer() {
	if (this->m_buffer != nullptr) {
		free(this->m_buffer);
	}
}

void
ring_buffer::push_back(const uint8_t *buf, int32_t size) {
	std::lock_guard<std::mutex> lock(this->m_mutex);

	if (size > this->_bytes_left()) {
		this->_ensure_capacity(size - this->_bytes_left());
	}

	int32_t _buf_bytes_left = this->m_buffer_size - this->m_write_offs;
	if (size < _buf_bytes_left) {
		memcpy(this->m_buffer + this->m_write_offs, buf, size);
		this->m_write_offs += size;
	} else if (size > _buf_bytes_left) {
		memcpy(this->m_buffer + this->m_write_offs, buf, _buf_bytes_left);
		this->m_write_offs = size - _buf_bytes_left;
		memcpy(this->m_buffer, buf + _buf_bytes_left, this->m_write_offs);
	} else {
		memcpy(this->m_buffer + this->m_write_offs, buf, size);
		this->m_write_offs = 0;
	}

	this->m_bytes_written += size;

	/*int32_t size_left = size;
	int32_t dest_size = std::min(this->m_buffer_size - this->m_write_offs,
	size_left); memcpy(this->m_buffer + this->m_write_offs, buf, dest_size);
	size_left -= dest_size;
	this->m_write_offs = (this->m_write_offs + dest_size) % this->m_buffer_size;

	// round robin and write again
	if (size_left > 0) {
	  memcpy(this->m_buffer, buf + dest_size, size_left);
	  this->m_write_offs = (this->m_write_offs + size_left) % this->m_buffer_size;
	}*/

	// naive approach
	/*for (int32_t i = 0; i < size; ++i) {
	  *(this->m_buffer + this->m_write_offs) = buf[i];
	  this->m_write_offs = (this->m_write_offs + 1) % this->m_buffer_size;
	}*/
}

int32_t
ring_buffer::pop_front(uint8_t *buf, int32_t size) {
	std::lock_guard<std::mutex> lock(this->m_mutex);
	return this->_pop_front(buf, size);
}

int32_t
ring_buffer::pop_front(int32_t size) {
	std::lock_guard<std::mutex> lock(this->m_mutex);

	int32_t rcvd = std::min(size, this->_bytes_written());
	if (rcvd <= 0) {
		return 0;
	}

	this->m_read_offs = (this->m_read_offs + rcvd) % this->m_buffer_size;
	this->m_bytes_written -= rcvd;

	return rcvd;
}

int32_t
ring_buffer::size() {
	std::lock_guard<std::mutex> lock(this->m_mutex);
	return this->_bytes_written();
}

int32_t
ring_buffer::_bytes_left() {
	return this->m_buffer_size - this->_bytes_written();
}

int32_t
ring_buffer::_bytes_written() {
	return this->m_bytes_written;
}

void
ring_buffer::_ensure_capacity(int32_t size) {
	if (this->m_buffer == nullptr) {
		this->m_buffer = (uint8_t *)malloc(size);
		this->m_buffer_size = size;
	} else {
		this->m_buffer = (uint8_t *)realloc(this->m_buffer, this->m_buffer_size + size);
		this->m_write_offs += this->m_buffer_size;
		this->m_buffer_size += size;
	}
}

int32_t
ring_buffer::_pop_front(uint8_t *buf, int32_t size) {
	int32_t rcvd = std::min(size, this->_bytes_written());
	if (rcvd <= 0) {
		return 0;
	}

	int32_t _buf_bytes_left = this->m_buffer_size - this->m_read_offs;
	if (rcvd < _buf_bytes_left) {
		memcpy(buf, this->m_buffer + this->m_read_offs, rcvd);
		this->m_read_offs += rcvd;
	} else if (rcvd > _buf_bytes_left) {
		memcpy(buf, this->m_buffer + this->m_read_offs, _buf_bytes_left);
		this->m_read_offs = rcvd - _buf_bytes_left;
		memcpy(buf + _buf_bytes_left, this->m_buffer, this->m_read_offs);
	} else {
		memcpy(buf, this->m_buffer + this->m_read_offs, rcvd);
		this->m_read_offs = 0;
	}

	this->m_bytes_written -= rcvd;

	// initial read to the end of the buffer
	/*int32_t size_left = size;
	int32_t dest_size = std::min(this->m_buffer_size - this->m_read_offs,
	size_left); memcpy(buf, this->m_buffer + this->m_read_offs, dest_size);
	size_left -= dest_size;
	this->m_read_offs = (this->m_read_offs + dest_size) % this->m_buffer_size;

	// round robin and read again
	if (size_left > 0) {
	  memcpy(buf + dest_size, this->m_buffer, size_left);
	  this->m_read_offs = (this->m_read_offs + size_left) % this->m_buffer_size;
	}*/

	// naive approach
	/*for (int32_t i = 0; i < size; ++i) {
	  buf[i] = *(this->m_buffer + this->m_read_offs);
	  this->m_read_offs = (this->m_read_offs + 1) % this->m_buffer_size;
	}*/

	return rcvd;
}
