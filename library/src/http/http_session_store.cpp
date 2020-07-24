//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/botan.h>
#include <networkio/http/http_session_store.h>

using namespace networkio;
using namespace networkio::http;

//----------------------------------------------------------------------------
// networkio::http::session
//----------------------------------------------------------------------------

session::session(void) {}

session::session(const std::string &sid) { this->m_session_id = sid; }

//----------------------------------------------------------------------------
// networkio::http::session_store
//----------------------------------------------------------------------------

session_store::session_store(void) {}

std::shared_ptr<session>
session_store::get(connection &conn) {
	auto &req = conn.request();
	auto sid = req.cookies()["session_id"];

	// TODO: check if sid is empty to prevent locking

	this->m_mutex.lock_shared();
	auto s = this->m_sessions[sid]; // TODO use .get() for thread safety
	this->m_mutex.unlock_shared();

	// create new session, lock in exclusive state
	if (s == nullptr || !s->is_valid()) {
		this->m_mutex.lock();

		// search for a free session_id
		while (true) {
			sid = this->generate_session_id();
			s = this->m_sessions[sid];
			if (s == nullptr || !s->is_valid()) {
				s = std::make_shared<session>(sid);
				break;
			}
		}

		this->m_sessions[sid] = s;
		this->m_mutex.unlock();

		// set session cookie
		auto &cookie = conn.response().cookies()["session_id"];
		cookie.set_name("session_id");
		cookie.set_value(sid);
		cookie.set_http_only(true);
		cookie.set_path("/");

		return s;
	}

	// valid session found, return it
	return s;
}

std::string
session_store::generate_session_id(void) {
	// TODO: implement me
	/*
	std::unique_ptr<Botan::RandomNumberGenerator> rng;
#if defined(BOTAN_HAS_SYSTEM_RNG)
	rng.reset(new Botan::System_RNG);
#elif defined(BOTAN_HAS_AUTO_SEEDING_RNG)
	rng.reset(new Botan::AutoSeeded_RNG);
#endif

	// TODO: throw?
	if (rng == nullptr) {
		return "";
	}

	uint8_t buffer[32];
	rng->randomize(buffer, sizeof(buffer));

	// hash it
	std::unique_ptr<Botan::HashFunction> sha1(Botan::HashFunction::create("SHA-1"));
	sha1->update(buffer, sizeof(buffer));
	return Botan::hex_encode(sha1->final());
	*/
	return "implement_me";
}
