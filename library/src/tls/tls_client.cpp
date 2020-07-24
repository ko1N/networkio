#ifdef __WITH_BOTAN__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/stringutil.h>
#include <networkio/tls/tls_client.h>

#include <functional>
#include <list>

using namespace networkio::tls;
using namespace networkio::socket;

//----------------------------------------------------------------------------
// networkio::tls::client
//----------------------------------------------------------------------------

client::client(rng_type rng) { this->m_rng_type = rng; }

client::client(std::shared_ptr<networkio::interfaces::client> sock, server *sv, size_t bufsize) {
	this->m_client = sock;
	this->m_tls_channel.reset(new Botan::TLS::Server(*this, *sv->m_session_manager.get(),
													 *sv->m_credential_manager.get(), *sv->m_policy.get(), sv->rng(),
													 false, bufsize));
	this->finish_connection();
}

client::~client() {
	this->close();

	if (this->m_client != nullptr) {
		this->m_client->close();
	}
}

bool
client::set_certificate(const std::string &cert) {
	this->m_cert = cert;
	return true;
}

bool
client::set_buffer_size(size_t bufsize) {
	this->m_bufsize = bufsize;
	return true;
}

// TODO: this is not the best design and we can probably get rid of this func
bool
client::create_socket(void) {
	return false;
}

bool
client::set_blocking(bool block) {
	if (this->m_client == nullptr)
		return false;

	return this->m_client->set_blocking(block);
}

bool
client::set_nopipe(bool nopipe) {
	if (this->m_client == nullptr)
		return false;

	return this->m_client->set_nopipe(nopipe);
}

bool
client::connect(const std::string &addr) {
	// delete socket if necessary
	if (this->m_client != nullptr) {
		this->m_client->close();
	}

	// create new TCP client
	this->m_client = std::make_shared<tcp_client>();
	if (!this->m_client->connect(addr)) {
		this->m_client->close();
		this->m_client = nullptr;
		return false;
	}

	/*
	if (!client->set_blocking(false)) {
			delete client;
			return false;
	}
	*/

	// clear buffers
	this->m_in_buffer.clear();
	this->m_out_buffer.clear();

	// create tls client
	this->m_tls_channel = nullptr; // early deletion

	this->m_session_manager.reset(new Botan::TLS::Session_Manager_In_Memory(this->rng()));
	if (this->m_cert != "") {
		// printf("tls::client::connect(): cert pinning enabled.\n");
		this->m_credential_manager.reset(new credentials_manager(this->m_cert));
	} else {
		// printf("tls::client::connect(): no cert set. cert pinning is disabled.\n");
		this->m_credential_manager.reset(new credentials_manager());
	}

	this->m_policy.reset(new Botan::TLS::Strict_Policy);

	this->m_tls_channel.reset(
		new Botan::TLS::Client(*this, *this->m_session_manager.get(), *this->m_credential_manager.get(),
							   *this->m_policy.get(), this->rng(), Botan::TLS::Server_Information("libnetworkio"),
							   this->m_policy->latest_supported_version(false), {}, this->m_bufsize));

	return this->finish_connection();
}

bool
client::finish_connection(void) {
	while (!this->m_tls_channel->is_closed() && !this->m_tls_channel->is_active()) {
		if (!this->_send() || !this->_recv()) {
			printf("%s: failed to connect during handshake.\n", __FUNCTION__);
			this->close();
			return false;
		}
	}

	return true;
}

bool
client::is_connected(void) {
	if (this->m_client == nullptr || this->m_tls_channel == nullptr) {
		return false;
	}

	return this->m_client->is_connected() && this->m_tls_channel->is_active();
}

bool
client::is_closed(void) {
	if (this->m_client == nullptr || this->m_tls_channel == nullptr) {
		return true;
	}

	return !this->m_client->is_connected() || this->m_tls_channel->is_closed();
}

bool
client::close(void) {
	if (this->m_tls_channel == nullptr) {
		return false;
	}

	try {
		this->m_tls_channel->close();
	} catch (const Botan::Exception &e) {
		// printf("exception in botan: %s.\n", e.what());
		return false;
	}

	return true;
}

int32_t
client::read_raw(uint8_t *buf, int32_t len) {
	if (this->is_closed() || !this->_recv()) {
		return -1;
	}

	return this->m_in_buffer.pop_front(buf, len);
}

int32_t
client::write_raw(const uint8_t *buf, int32_t len) {
	if (this->is_closed()) {
		printf("tls::client::write_raw(): connection closed.\n");
		return -1;
	}

	if (!this->is_connected()) {
		return 0;
	}

	try {
		this->m_tls_channel->send(buf, len);
	} catch (const Botan::Exception &e) {
		// printf("exception in botan: %s.\n", e.what());
		this->close();
		return -1;
	}

	return len;
}

bool
client::_recv(void) {
	if (this->m_client == nullptr || !this->m_client->is_connected()) {
		return false;
	}

	uint8_t *_buf = (uint8_t *)malloc(this->m_bufsize);
	int rcvd = this->m_client->read_raw(_buf, this->m_bufsize);
	if (rcvd < 0) {
		free(_buf);
		this->close();
		return false; // disconnected
	}

	if (rcvd > 0) {
		try {
			this->m_tls_channel->received_data(_buf, rcvd);
		} catch (const Botan::Exception &e) {
			// printf("exception in botan: %s.\n", e.what());
			free(_buf);
			this->close();
			return false; // disconnected
		}
	}

	free(_buf);
	return true;
}

bool
client::_send(void) {
	if (this->m_client == nullptr || !this->m_client->is_connected()) {
		return false;
	}

	int32_t size = this->m_out_buffer.size();
	uint8_t *buf = (uint8_t *)malloc(size);
	this->m_out_buffer.pop_front(buf, size);

	int sent = this->m_client->write_raw(buf, size);
	if (sent < 0) {
		free(buf);
		this->close();
		return false; // disconnected
	}

	this->m_out_buffer.push_back(buf + sent, size - sent);
	free(buf);

	return true;
}

Botan::RandomNumberGenerator &
client::rng(void) {
	if (this->m_rng == nullptr) {
		if (this->m_rng_type == RNG_System) {
#if defined(BOTAN_HAS_SYSTEM_RNG)
			this->m_rng.reset(new Botan::System_RNG);
#endif
		} else if (this->m_rng_type == RNG_Auto) {
#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
			this->m_rng.reset(new Botan::AutoSeeded_RNG);
#endif
		}
	}

	return *this->m_rng.get();
}

void
client::tls_verify_cert_chain(const std::vector<Botan::X509_Certificate> &cert_chain,
							  const std::vector<std::shared_ptr<const Botan::OCSP::Response>> &ocsp,
							  const std::vector<Botan::Certificate_Store *> &trusted_roots, Botan::Usage_Type usage,
							  const std::string &hostname, const Botan::TLS::Policy &policy) {
	// printf("client::tls_verify_cert_chain()\n");

	// TODO: this should only be checked on the client (cert pinning);
	/*
	if (cert_chain.empty())
			throw std::invalid_argument("certificate chain was empty");

	if (!trusted_roots[0]->certificate_known(cert_chain[0]))
			throw std::runtime_error("unable to establish trust");
	*/

	/*
	Botan::Path_Validation_Restrictions
	restrictions(policy.require_cert_revocation_info(),
	policy.minimum_signature_strength());

	auto ocsp_timeout = std::chrono::milliseconds(1000);
	Botan::Path_Validation_Result result =
			Botan::x509_path_validate(cert_chain,
					restrictions,
					trusted_roots,
					hostname,
					usage,
					std::chrono::system_clock::now(),
					ocsp_timeout,
					ocsp);

	std::cout << "Certificate validation status: " << result.result_string() <<
	"\n"; if (result.successful_validation())
	{
			auto status = result.all_statuses();

			if (status.size() > 0 &&
	status[0].count(Botan::Certificate_Status_Code::OCSP_RESPONSE_GOOD)) std::cout
	<< "Valid OCSP response for this server\n";
	}
	else
	{
			throw std::exception("unable to establish trust");
	}
	*/
}

bool
client::tls_session_established(const Botan::TLS::Session &session) {
	return true;
}

void
client::tls_emit_data(const uint8_t buf[], size_t length) {
	this->m_out_buffer.push_back(buf, length);
	this->_send();
}

void
client::tls_alert(Botan::TLS::Alert alert) {
	if (alert.type() == alert.CLOSE_NOTIFY) {
		this->m_client->close();
		this->m_client = nullptr;
	}

	printf("%s: %s\n", __FUNCTION__, alert.type_string().c_str());
}

void
client::tls_record_received(uint64_t seq_no, const uint8_t buf[], size_t buf_size) {
	this->m_in_buffer.push_back(buf, buf_size);
}

#endif
