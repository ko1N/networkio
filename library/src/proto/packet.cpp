
//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <ios>

#include <networkio/memory/shared_buffer.h>
#include <networkio/proto/packet.h>

using namespace networkio::proto;

//----------------------------------------------------------------------------
// networkio::proto::packet
//----------------------------------------------------------------------------

packet::packet(SOCKADDR_IN *address) {
	if (address != nullptr) {
		memcpy(&this->m_address, address, sizeof(SOCKADDR_IN));
	} else {
		memset(&this->m_address, 0, sizeof(SOCKADDR_IN));
	}

	this->ensure_capacity(4096);
}

packet::packet(networkio::memory::shared_buffer buffer, SOCKADDR_IN *address) {
	this->m_buffer = buffer;
	this->m_read_pointer = const_cast<uint8_t *>(this->m_buffer->data());
	this->m_write_pointer = const_cast<uint8_t *>(this->m_buffer->data()) + this->m_buffer->size();

	if (address != nullptr) {
		memcpy(&this->m_address, address, sizeof(SOCKADDR_IN));
	} else {
		memset(&this->m_address, 0, sizeof(SOCKADDR_IN));
	}
}

packet::packet(const packet &p) {
	if (p.m_buffer != nullptr) {
		this->m_buffer = std::make_shared<memory::buffer>(*p.m_buffer.get());
		this->m_read_pointer = const_cast<uint8_t *>(this->m_buffer->data());
		this->m_write_pointer = const_cast<uint8_t *>(this->m_buffer->data()) + this->m_buffer->size();
	}
	memcpy(&this->m_address, &p.m_address, sizeof(SOCKADDR_IN));
}

packet::~packet() {}

uint32_t
packet::read_bytesleft(void) const {
	if (this->m_buffer == nullptr || this->m_read_pointer == nullptr) {
		return 0;
	}

	return (this->size() - (uint32_t)(this->m_read_pointer - this->m_buffer->data()));
}

std::string
packet::read_string(void) {
	uint32_t length = this->read<uint32_t>();
	if (length == 0) {
		return "";
	}

	char *buffer = (char *)malloc(length + 1);
	if (this->read_data((uint8_t *)buffer, length)) {
		buffer[length] = '\0'; // append missing 0 terminator
		std::string str = std::string(buffer, length);
		free(buffer);
		return str;
	} else {
		return "";
	}
}

void
packet::ensure_capacity(uint32_t length) {
	if (this->m_buffer == nullptr) {
		this->m_buffer = std::make_shared<memory::buffer>(length);
		this->m_read_pointer = const_cast<uint8_t *>(this->m_buffer->data());
		this->m_write_pointer = const_cast<uint8_t *>(this->m_buffer->data());
	} else {
		if ((this->m_buffer->size() - (uint32_t)(this->m_write_pointer - this->m_buffer->data())) < length) {
			// TODO: fix read_buffer here in case we read and write to the same
			// packet!
			uint32_t old_size = this->size();
			this->m_buffer->ensure_capacity(length);
			this->m_write_pointer = const_cast<uint8_t *>(this->m_buffer->data()) + old_size;

			/*
			uint8_t *buffer = (uint8_t *)malloc(this->m_length + length);
			memcpy(buffer, this->m_buffer, this->m_length);

			this->m_write_buffer = buffer + this->m_length;
			this->m_length += length;

			free(this->m_buffer);
			this->m_buffer = buffer;
			*/
		}
	}
}

void
packet::write_string(std::string str) {
	this->write((uint32_t)str.length()); // we force to write an uint32 here
	this->write_data((uint8_t *)str.c_str(), str.length());
}

std::string
packet::to_string(void) {
	if (this->m_buffer == nullptr) {
		return "";
	}
	return this->m_buffer->to_string();
}
