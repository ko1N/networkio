
#ifndef __SOCKET_TCP_H__
#define __SOCKET_TCP_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/socket/socket.h>
#include <networkio/types.h>

#include <networkio/interfaces/client.h>
#include <networkio/interfaces/server.h>

#include <deque>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace socket {

//----------------------------------------------------------------------------
// networkio::socket::tcp_socket
//----------------------------------------------------------------------------

class tcp_socket : public socket {

  public:
	tcp_socket(void);
	virtual ~tcp_socket();

  public:
	virtual bool
	is_stream(void) override {
		return true;
	}
	virtual bool create_socket(void) override;
};

//----------------------------------------------------------------------------
// networkio::socket::tcp_server
//----------------------------------------------------------------------------

class tcp_server : public tcp_socket, public virtual interfaces::server {

  public:
	tcp_server(void);
	virtual ~tcp_server();

  public:
	virtual bool bind(u_short uPort) override;
	virtual std::shared_ptr<interfaces::client> accept(struct sockaddr_in *addr = nullptr) override;
};

//----------------------------------------------------------------------------
// networkio::socket::tcp_client
//----------------------------------------------------------------------------

class tcp_client : public tcp_socket, public virtual interfaces::client {

	friend class tcp_server;

  public:
	tcp_client(void);
	virtual ~tcp_client();

  public:
	virtual bool connect(const std::string &addr) override;
	virtual bool is_connected(void) override;
	virtual bool close(void) override;

  protected:
	bool write_raw_blocked(const uint8_t *buf, int32_t len);

  public:
	virtual int32_t read_raw(uint8_t *buf, int32_t len) override;
	virtual int32_t write_raw(const uint8_t *buf, int32_t len) override;

  protected:
	bool m_connected = false;
};

} // namespace socket
} // namespace networkio

#endif
