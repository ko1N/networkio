#ifdef __WITH_BOTAN__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <functional>
#include <list>

#include <networkio/tls/tls_client.h>
#include <networkio/tls/tls_server.h>

using namespace networkio::tls;

//----------------------------------------------------------------------------
// networkio::tls::server
//----------------------------------------------------------------------------

server::server(rng_type rng) { this->m_rng_type = rng; }

server::~server() {
	if (this->m_server != nullptr) {
		this->m_server->close();
	}
}

bool
server::set_certificate(const std::string &cert, const std::string &priv_key, const std::string &priv_key_pwd) {
	this->m_credential_manager.reset(new credentials_manager(cert, this->rng(), priv_key, priv_key_pwd));
	return true;
}

bool
server::set_buffer_size(size_t bufsize) {
	this->m_bufsize = bufsize;
	return true;
}

// TODO: this is not the best design and we can probably get rid of this func
bool
server::create_socket(void) {
	return false;
}

bool
server::set_blocking(bool block) {
	if (this->m_server == nullptr)
		return false;

	return this->m_server->set_blocking(block);
}

bool
server::set_nopipe(bool nopipe) {
	if (this->m_server == nullptr)
		return false;

	return this->m_server->set_nopipe(nopipe);
}

bool
server::bind(const u_short port) {
	// delete socket if necessary
	if (this->m_server != nullptr) {
		this->m_server->close();
		this->m_server = nullptr;
	}

	// prerequesities
	this->m_policy.reset(new Botan::TLS::Strict_Policy);
	this->m_session_manager.reset(new Botan::TLS::Session_Manager_In_Memory(this->rng()));

	// create tcp server
	// TODO: make shared
	this->m_server = std::make_shared<networkio::socket::tcp_server>();
	if (!this->m_server->bind(port)) {
		this->m_server = nullptr;
		return false;
	}

	/*
	if (!pSocket->SetBlocking(false)) {
			delete pSocket;
			return false;
	}
	*/

	return true;
}

std::shared_ptr<networkio::interfaces::client>
server::accept(struct sockaddr_in *addr) {
	struct sockaddr_in a;
	auto sock = this->m_server->accept(&a);
	if (sock == nullptr) {
		return nullptr;
	}

	if (addr != nullptr) {
		memcpy(addr, &a, sizeof(struct sockaddr_in));
	}

	// TODO: do we want to store all clients?!
	return std::make_shared<client>(sock, this, this->m_bufsize);
}

bool
server::close(void) {
	if (this->m_server == nullptr)
		return false;

	// TODO: we might want to force close all client connections here

	return this->m_server->close();
}

Botan::RandomNumberGenerator &
server::rng(void) {
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

#endif
