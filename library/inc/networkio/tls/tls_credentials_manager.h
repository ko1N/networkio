
#ifndef __TLS_CREDENTIALSMANAGER_H__
#define __TLS_CREDENTIALSMANAGER_H__
#ifdef __WITH_BOTAN__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/botan.h>

#include <botan/credentials_manager.h>
#include <botan/data_src.h>
#include <botan/pkcs8.h>
#include <botan/x509self.h>

#include <fstream>
#include <iostream>
#include <memory>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace tls {

//----------------------------------------------------------------------------
// networkio::tls::credentials_manager
//----------------------------------------------------------------------------

class credentials_manager : public Botan::Credentials_Manager {
  public:
	credentials_manager(void);
	credentials_manager(const std::string &cert);
	credentials_manager(const std::string &cert, Botan::RandomNumberGenerator &rng, const std::string &priv_key,
						const std::string &priv_key_pwd);

  public:
	std::vector<Botan::Certificate_Store *> trusted_certificate_authorities(const std::string &type,
																			const std::string &host) override;

	std::vector<Botan::X509_Certificate> cert_chain(const std::vector<std::string> &algos, const std::string &type,
													const std::string &host) override;

	Botan::Private_Key *private_key_for(const Botan::X509_Certificate &cert, const std::string &type,
										const std::string &ctx) override;

  private:
	struct certificate_info {
		std::vector<Botan::X509_Certificate> certs;
		std::shared_ptr<Botan::Private_Key> key;
	};

	std::vector<certificate_info> m_certs;
	std::vector<std::shared_ptr<Botan::Certificate_Store>> m_cert_stores;
};

} // namespace tls
} // namespace networkio

#endif
#endif
