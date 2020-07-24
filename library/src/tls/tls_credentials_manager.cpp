#ifdef __WITH_BOTAN__

#include <networkio/tls/tls_credentials_manager.h>

using namespace networkio::tls;

credentials_manager::credentials_manager(void) {}

credentials_manager::credentials_manager(const std::string &cert) {
	std::shared_ptr<Botan::DataSource_Memory> cds;
	cds.reset(new Botan::DataSource_Memory(cert));

	std::shared_ptr<Botan::X509_Certificate> x509;
	x509.reset(new Botan::X509_Certificate(*cds.get()));

	std::shared_ptr<Botan::Certificate_Store> cert_store;
	cert_store.reset(new Botan::Certificate_Store_In_Memory(*x509.get()));
	this->m_cert_stores.push_back(cert_store);
}

credentials_manager::credentials_manager(const std::string &cert, Botan::RandomNumberGenerator &rng,
										 const std::string &priv_key, const std::string &priv_key_pwd) {
	std::shared_ptr<Botan::DataSource_Memory> cds;
	cds.reset(new Botan::DataSource_Memory(cert));

	std::shared_ptr<Botan::X509_Certificate> x509;
	x509.reset(new Botan::X509_Certificate(*cds.get()));

	// store cert as store
	std::shared_ptr<Botan::Certificate_Store> cert_store;
	cert_store.reset(new Botan::Certificate_Store_In_Memory(*x509.get()));
	this->m_cert_stores.push_back(cert_store);

	// store private key
	certificate_info ci;
	Botan::DataSource_Memory pkds(priv_key);
	ci.key.reset(Botan::PKCS8::load_key(pkds, rng, priv_key_pwd));
	ci.certs.push_back(*x509.get());

	this->m_certs.push_back(ci);
}

std::vector<Botan::Certificate_Store *>
credentials_manager::trusted_certificate_authorities(const std::string &type, const std::string &host) {
	std::vector<Botan::Certificate_Store *> v;

	// don't ask for client certs
	if (type == "tls-server")
		return v;

	for (auto &&cs : this->m_cert_stores)
		v.push_back(cs.get());

	return v;
}

std::vector<Botan::X509_Certificate>
credentials_manager::cert_chain(const std::vector<std::string> &algos, const std::string &type,
								const std::string &host) {
	BOTAN_UNUSED(type);

	for (auto &&i : this->m_certs) {
		if (std::find(algos.begin(), algos.end(), i.key->algo_name()) == algos.end())
			continue;

		if (host != "" && !i.certs[0].matches_dns_name(host))
			continue;

		return i.certs;
	}

	return std::vector<Botan::X509_Certificate>();
}

Botan::Private_Key *
credentials_manager::private_key_for(const Botan::X509_Certificate &cert, const std::string &type,
									 const std::string &ctx) {
	for (auto &&i : this->m_certs) {
		if (cert == i.certs[0]) {
			return i.key.get();
		}
	}

	return nullptr;
}

#endif
