
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/memory/shared_buffer.h>

#include <inttypes.h>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

using namespace networkio::memory;

//----------------------------------------------------------------------------
// networkio::memory::shared_buffer
//----------------------------------------------------------------------------

buffer::buffer(uint32_t size) {
	this->m_buffer = (uint8_t *)malloc(size);
	this->m_begin = this->m_buffer;
	this->m_size = size;
}

buffer::buffer(const buffer &o) {
	this->m_buffer = (uint8_t *)malloc(o.m_size);
	memcpy(this->m_buffer, o.m_buffer, o.m_size);
	this->m_begin = this->m_buffer + o.m_offset;
	this->m_offset = o.m_offset;
	this->m_size = o.m_size;
}

buffer::~buffer() {
	if (this->m_buffer != nullptr) {
		free(this->m_buffer);
	}
}

const uint8_t *
buffer::data() const {
	return this->m_begin;
}

const uint32_t
buffer::size() const {
	return this->m_size - this->m_offset;
}

void
buffer::ensure_capacity(uint32_t size) {
	if (this->m_buffer == nullptr) {
		this->m_buffer = (uint8_t *)malloc(size);
		this->m_begin = this->m_buffer + this->m_offset;
	} else {
		this->m_buffer = (uint8_t *)realloc(this->m_buffer, this->m_size + size);
		this->m_begin = this->m_buffer + this->m_offset;
	}
	this->m_size += size;
}

void
buffer::set_offset(uint32_t offset) {
	if (this->m_buffer != nullptr && offset < this->m_size) {
		this->m_begin = this->m_buffer + offset;
		this->m_offset = offset;
	}
}

std::string
buffer::to_string() {
	std::stringstream ss;
	for (uint32_t i = 0; i < this->size(); i++) {
		ss << std::hex << (unsigned int)(this->m_begin[i]);
	}
	return ss.str();
}
