
#ifndef __SOCKET_UDP_H__
#define __SOCKET_UDP_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/socket/socket.h>
#include <networkio/types.h>

#include <deque>
#include <mutex>

#include <networkio/interfaces/client.h>
#include <networkio/interfaces/server.h>
#include <networkio/memory/fifo_buffer.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace socket {

//----------------------------------------------------------------------------
// networkio::socket::udp_socket
//----------------------------------------------------------------------------

class udp_socket : public socket {

  public:
	udp_socket(void);
	virtual ~udp_socket();

  public:
	virtual bool
	is_stream(void) override {
		return false;
	}
	virtual bool create_socket(void) override;
};

//----------------------------------------------------------------------------
// networkio::socket::udp_server
//----------------------------------------------------------------------------

class udp_client;
class udp_server : public udp_socket, public virtual interfaces::server {

  public:
	udp_server(void);
	virtual ~udp_server();

  public:
	virtual bool bind(u_short port) override;
	virtual std::shared_ptr<interfaces::client> accept(struct sockaddr_in *addr = nullptr) override;

  public:
	void _recv(void);

  protected:
	// TODO: implementing a unordered map with custom hashing would be p
	std::mutex m_mutex;
	using client_t = std::pair<sockaddr_in, std::shared_ptr<udp_client>>;
	std::vector<client_t> m_clients;
	std::deque<client_t> m_new_clients;
};

//----------------------------------------------------------------------------
// networkio::socket::udp_client
//----------------------------------------------------------------------------

class udp_client : public udp_socket, public virtual interfaces::client {

	friend class udp_server;

  public:
	udp_client(void);
	udp_client(udp_server *server, SOCKET sockfd, SOCKADDR_IN *addr);
	virtual ~udp_client();

  public:
	virtual bool connect(const std::string &addr) override;
	virtual bool is_connected(void) override;
	virtual bool close(void) override;

  public:
	virtual int32_t read_raw(uint8_t *buf, int32_t size) override;
	virtual int32_t write_raw(const uint8_t *buf, int32_t size) override;

  protected:
	void _recv(const uint8_t *buf, int32_t size);

  protected:
	// connet
	SOCKADDR_IN m_addr;
	bool m_connected = false;
	udp_server *m_server = nullptr;
	bool m_should_recv = true;

	networkio::memory::fifo_buffer m_buffer;
};

} // namespace socket
} // namespace networkio

#endif
