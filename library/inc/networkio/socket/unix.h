
#ifndef __SOCKET_UNIX_H__
#define __SOCKET_UNIX_H__
#ifndef _WIN32

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/socket/socket.h>
#include <networkio/types.h>

#include <networkio/interfaces/client.h>
#include <networkio/interfaces/server.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace socket {

//----------------------------------------------------------------------------
// networkio::socket::unix_socket
//----------------------------------------------------------------------------

class unix_socket : public socket {

  public:
	unix_socket();
	virtual ~unix_socket();

  public:
	virtual bool
	is_stream() override {
		return true;
	}
	virtual bool create_socket() override;
};

//----------------------------------------------------------------------------
// networkio::socket::unix_server
//----------------------------------------------------------------------------

class unix_server : public unix_socket, public virtual interfaces::server {

  public:
	unix_server();
	virtual ~unix_server();

  public:
	virtual bool bind(u_short port) override;
	virtual std::shared_ptr<interfaces::client> accept(struct sockaddr_in *addr = nullptr) override;
	virtual bool close() override;

  protected:
	std::string m_path = "";
};

//----------------------------------------------------------------------------
// networkio::socket::unix_client
//----------------------------------------------------------------------------

class unix_client : public unix_socket, public virtual interfaces::client {

	friend class unix_server;

  public:
	unix_client();
	virtual ~unix_client();

  public:
	virtual bool connect(const std::string &addr) override;
	virtual bool is_connected() override;
	virtual bool close() override;

  public:
	virtual int32_t read_raw(uint8_t *buf, int32_t len) override;
	virtual int32_t write_raw(const uint8_t *buf, int32_t len) override;

  protected:
	bool m_connected = false;
};

} // namespace socket
} // namespace networkio

#endif
#endif
