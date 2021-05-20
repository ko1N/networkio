
#ifndef __TLS_SERVER_H__
#define __TLS_SERVER_H__
#ifdef __WITH_BOTAN__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/botan.h>
#include <networkio/tls/tls_credentials_manager.h>

#include <networkio/socket/socket.h>
#include <networkio/socket/tcp.h>
#include <networkio/types.h>

#include <string>

#include <networkio/interfaces/server.h>

#include <networkio/tls/tls_types.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace tls {

//----------------------------------------------------------------------------
// networkio::tls::server
//----------------------------------------------------------------------------

class server : public virtual networkio::interfaces::server {

	friend class client;

  public:
	server(rng_type rng = RNG_System);
	~server();

  public:
	// options
	bool set_certificate(const std::string &cert, const std::string &priv_key, const std::string &priv_key_pwd);
	bool set_buffer_size(size_t bufsize = Botan::TLS::Channel::IO_BUF_DEFAULT_SIZE);

	// interface implementations
	virtual bool
	is_stream() override {
		return true;
	}
	virtual bool create_socket() override;
	virtual bool set_blocking(bool block) override;
	virtual bool set_nopipe(bool nopipe) override;

	virtual bool bind(const u_short port) override;
	virtual std::shared_ptr<networkio::interfaces::client> accept(struct sockaddr_in *addr = nullptr) override;

	virtual bool close() override;

  protected:
	Botan::RandomNumberGenerator &rng();

  protected:
	rng_type m_rng_type;
	std::unique_ptr<Botan::RandomNumberGenerator> m_rng;
	std::unique_ptr<Botan::TLS::Session_Manager_In_Memory> m_session_manager;
	std::unique_ptr<credentials_manager> m_credential_manager;
	std::unique_ptr<Botan::TLS::Policy> m_policy;
	std::shared_ptr<networkio::interfaces::server> m_server;
	size_t m_bufsize = Botan::TLS::Channel::IO_BUF_DEFAULT_SIZE;
};

} // namespace tls
} // namespace networkio

#endif
#endif
