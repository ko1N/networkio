
#ifndef __TLS_CLIENT_H__
#define __TLS_CLIENT_H__
#ifdef __WITH_BOTAN__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/botan.h>
#include <networkio/interfaces/client.h>
#include <networkio/memory/fifo_buffer.h>
#include <networkio/socket/socket.h>
#include <networkio/socket/tcp.h>
#include <networkio/tls/tls_credentials_manager.h>
#include <networkio/tls/tls_server.h>
#include <networkio/tls/tls_types.h>
#include <networkio/types.h>

#include <string>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace tls {

//----------------------------------------------------------------------------
// networkio::tls::client
//----------------------------------------------------------------------------

class client : public virtual networkio::interfaces::client, public Botan::TLS::Callbacks {

	friend class server;

  public:
	client(rng_type rng = RNG_System);
	client(std::shared_ptr<networkio::interfaces::client> sock, server *sv,
		   size_t bufsize = Botan::TLS::Channel::IO_BUF_DEFAULT_SIZE);
	~client();

  public:
	// options
	bool set_certificate(const std::string &cert);
	bool set_buffer_size(size_t bufsize = Botan::TLS::Channel::IO_BUF_DEFAULT_SIZE);

	// client interfaces
	virtual bool
	is_stream() override {
		return true;
	}
	virtual bool create_socket() override;
	virtual bool set_blocking(bool block) override;
	virtual bool set_nopipe(bool nopipe) override;

	virtual bool connect(const std::string &addr) override;
	virtual bool finish_connection();

	bool is_connected() override;
	bool is_closed();

	virtual int32_t read_raw(uint8_t *buf, int32_t len) override;
	virtual int32_t write_raw(const uint8_t *buf, int32_t len) override;

	virtual bool close() override;

  protected:
	bool _recv();
	bool _send();

  protected:
	Botan::RandomNumberGenerator &rng();

  protected:
	void tls_verify_cert_chain(const std::vector<Botan::X509_Certificate> &cert_chain,
							   const std::vector<std::shared_ptr<const Botan::OCSP::Response>> &ocsp,
							   const std::vector<Botan::Certificate_Store *> &trusted_roots, Botan::Usage_Type usage,
							   const std::string &hostname, const Botan::TLS::Policy &policy) override;
	bool tls_session_established(const Botan::TLS::Session &session) override;
	void tls_emit_data(const uint8_t buf[], size_t length) override;
	void tls_alert(Botan::TLS::Alert alert) override;
	void tls_record_received(uint64_t seq_no, const uint8_t buf[], size_t buf_size) override;

  protected:
	rng_type m_rng_type;
	std::unique_ptr<Botan::RandomNumberGenerator> m_rng;

	std::string m_cert;
	std::unique_ptr<Botan::TLS::Session_Manager_In_Memory> m_session_manager;
	std::unique_ptr<credentials_manager> m_credential_manager;
	std::unique_ptr<Botan::TLS::Policy> m_policy;

	std::shared_ptr<networkio::interfaces::client> m_client = nullptr;
	size_t m_bufsize = Botan::TLS::Channel::IO_BUF_DEFAULT_SIZE;
	std::unique_ptr<Botan::TLS::Channel> m_tls_channel;

	networkio::memory::fifo_buffer m_out_buffer;
	networkio::memory::fifo_buffer m_in_buffer;
};

} // namespace tls
} // namespace networkio

#endif
#endif
