
//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include <list>

#include <networkio/mux/mux_client.h>

using namespace networkio;
using namespace networkio::mux;
using namespace networkio::socket;

//----------------------------------------------------------------------------
// networkio::mux::client
//----------------------------------------------------------------------------

client::client() {}

client::client(std::shared_ptr<networkio::interfaces::client> cl) {
	this->m_client = std::move(cl);
	this->m_proto = std::make_shared<networkio::proto::packet_proto>(this->m_client);
}

client::~client() {}

void
client::set_auto_reconnect(int seconds) {
	this->m_auto_reconnect = seconds;
}

int
client::get_auto_reconnect() {
	return this->m_auto_reconnect;
}

bool
client::run(const std::string &address) {
	if (!this->connect(address)) {
		return false;
	}

	while (this->process()) {
		// TODO: implement a sleep option
	}

	return true;
}

bool
client::connect(const std::string &address) {
	if (this->m_client == nullptr) {
		this->m_client = std::make_shared<class tcp_client>();
	}

	if (!this->m_client->connect(address)) {
		this->m_connection_state = -1;
		return false;
	}

	this->m_client->set_blocking(false);
	this->m_client->set_nopipe(true);

	this->m_connection_state = 0;

	this->m_server = address;
	this->m_proto = std::make_shared<networkio::proto::packet_proto>(this->m_client);

	this->m_send_mutex.lock();
	this->m_sent_tp = std::chrono::high_resolution_clock::now();
	this->m_send_mutex.unlock();

	while (this->m_connection_state == 0) {
		this->process();
	}

	return true;
}

bool
client::is_connected() {
	return this->m_connection_state == 1;
}

bool
client::disconnect() {
	if (this->is_connected()) {
		auto ev = this->m_events[event_type::disconnect];
		if (ev != nullptr) {
			ev(this);
		}
	}
	this->m_connection_state = -1;
	return this->m_client->close();
}

bool
client::process() {
	if (this->m_proto == nullptr || !this->m_proto->process()) {

		this->handle_disconnect();

		// auto reconnect
		if (this->m_auto_reconnect > 0 && this->m_server != "") {
			if ((clock() - this->m_disconnect_time) / CLOCKS_PER_SEC > this->m_auto_reconnect) {
				if (this->connect(this->m_server)) {
					auto ev = this->m_events[event_type::reconnect];
					if (ev != nullptr) {
						ev(this);
					}
					return true;
				}

				this->m_disconnect_time = clock();
			}
		}

		return false;
	}

	// connect event
	if (this->m_client->is_connected()) {
		if (this->m_connection_state <= 0) {
			// setup connection state before in case we want to call() from the event
			this->m_connection_state = 1;
			auto ev = this->m_events[event_type::connect];
			if (ev != nullptr) {
				ev(this);
			}
		}
	}

	// did we receive new packets?
	while (true) {
		auto p = this->m_proto->read_packet();
		if (p == nullptr) {
			break;
		}

		// read endpoint
		hash::hash32_t endpoint = p->read<hash::hash32_t>();
		if (endpoint != MUX_HEARTBEAT && p->read_bytesleft() > 0) {
			if (this->m_endpoints.count(endpoint) > 0 && this->m_endpoints[endpoint] != nullptr) {
				p->set_start();
				this->m_endpoints[endpoint](this, p.get());
			} else {
				printf("%s: missing endpoint \"%d\"\n", __FUNCTION__, endpoint);
			}
		}
	}

	// check for timeout and send a heartbeat
	if (!this->check_heartbeat()) {
		return false;
	}

	// all ok
	return true;
}

void
client::event(event_type evt, std::function<void(client *)> &&func) {
	this->m_events[evt] = func;
}

bool
client::endpoint(const hash::hash32_t endpoint,
				 const std::function<void(client *, networkio::proto::packet *)> &&func) {
	// allow endpoint overwriting
	/*if (this->m_endpoints[endpoint] != nullptr) {
			printf("%s: endpoint is already defined.\n", __FUNCTION__);
			return false;
	}*/

	this->m_endpoints[endpoint] = func;
	// printf("%s: registered endpoint %x.\n", __FUNCTION__, endpoint);
	return true;
}

bool
client::call(const hash::hash32_t endpoint, const proto::packet *p) {
	if (!this->is_connected() || this->m_proto == nullptr) {
		return false;
	}

	// TODO: check buffer offset and write before it if possible
	// we will initialize the buffer with some memory left before/after it
	auto op = std::make_shared<proto::packet>();
	op->write<hash::hash32_t>(endpoint);
	op->write_data(p->buffer()->data(), p->size());
	if (!this->m_proto->write_packet(op)) {
		this->handle_disconnect();
		return false;
	}

	// update internal timer
	this->m_send_mutex.lock();
	this->m_sent_tp = std::chrono::high_resolution_clock::now();
	this->m_send_mutex.unlock();

	return true;
}

void
client::handle_disconnect() {
	// disconnect event
	if (this->m_connection_state > 0) {
		auto ev = this->m_events[event_type::disconnect];
		if (ev != nullptr) {
			ev(this);
		}

		this->m_client->close();
		this->m_disconnect_time = clock();
	}

	// set connection state
	this->m_connection_state = -1;
}

bool
client::check_heartbeat() {
	auto now = std::chrono::high_resolution_clock::now();
	this->m_send_mutex.lock();
	std::chrono::duration<double> elapsed = now - this->m_sent_tp;
	this->m_send_mutex.unlock();
	if (elapsed.count() > 1.0) {
		auto hb = std::make_shared<proto::packet>();
		hb->write<hash::hash32_t>(MUX_HEARTBEAT);
		if (!this->m_proto->write_packet(hb)) {
			this->handle_disconnect();
			return false;
		}

		this->m_send_mutex.lock();
		this->m_sent_tp = std::chrono::high_resolution_clock::now();
		this->m_send_mutex.unlock();
	}

	return true;
}