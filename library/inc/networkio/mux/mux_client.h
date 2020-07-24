
#ifndef __MUX_CLIENT_H__
#define __MUX_CLIENT_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/fnv1a.h>
#include <networkio/interfaces/client.h>
#include <networkio/proto/packet_proto.h>
#include <networkio/socket/concurrent_server.h>
#include <networkio/socket/socket.h>
#include <networkio/socket/tcp.h>
#include <networkio/stringutil.h>
#include <networkio/types.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <networkio/memory/user_data.h>
#include <networkio/mux/mux_memory.h>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace mux {

//----------------------------------------------------------------------------
// networkio::mux::event_type
//----------------------------------------------------------------------------

#define MUX_HEARTBEAT 0xFFFFFFFF

enum class event_type { connect = 0, disconnect, reconnect };

// this is clang specific...
struct enum_hash {
	template <typename T>
	std::size_t
	operator()(T t) const {
		return static_cast<std::size_t>(t);
	}
};

//----------------------------------------------------------------------------
// networkio::mux::client
//----------------------------------------------------------------------------

class client : public networkio::socket::tcp::concurrent_server_handler {

	friend class server;

  public:
	using callback_t = std::function<void(client *, networkio::proto::packet *)>;

  public:
	client(void);
	client(std::shared_ptr<networkio::interfaces::client> cl);
	~client();

  public:
	void set_auto_reconnect(int seconds);
	int get_auto_reconnect(void);

	bool run(std::string address); // this starts the client in blocking mode and
								   // processes all packets internally

	bool connect(std::string address); // this starts the client in non blocking mode
	bool is_connected(void);
	bool disconnect(void);
	bool process(void); // processes the server loop if using non blocking mode

	// event handlers
	void event(event_type evt, std::function<void(client *)> &&func);

	// simple endpoint communication
	bool endpoint(const hash::hash32_t endpoint, const callback_t &&func);
	bool call(const hash::hash32_t endpoint, const networkio::proto::packet *packet);

	// allocates a shared memory region and returns a pointer to it
	template <typename T>
	T *
	sharepoint(const hash::hash32_t sharepoint) {
		if (this->m_sharepoints[sharepoint] == nullptr) {
			this->m_sharepoints[sharepoint].reset(new memory<T>());
		}

		return this->m_sharepoints[sharepoint]->base();
	}

	// thread safe userdata
	// TODO: refactor using constexpr - where iss the thread safety here?
	template <typename T>
	std::shared_ptr<T>
	get_userdata(std::string userdata) {
		if (this->m_userdata[userdata] == nullptr) {
			this->m_userdata[userdata].reset(new T());
		}

		return std::static_pointer_cast<T>(this->m_userdata[userdata]);
	}

	template <typename T>
	std::shared_ptr<T>
	get_userdata(std::string userdata, std::function<T *()> &&func) {
		if (this->m_userdata[userdata] == nullptr) {
			this->m_userdata[userdata].reset(func());
		}

		return std::static_pointer_cast<T>(this->m_userdata[userdata]);
	}

  protected:
	void handle_disconnect(void);
	bool check_heartbeat(void);

	// TODO: i dont think thats super neat here...
	std::mutex &
	mutex(void) {
		return this->m_mutex;
	}

  protected:
	int m_auto_reconnect = 0;

	std::shared_ptr<networkio::interfaces::client> m_client;
	std::shared_ptr<networkio::proto::packet_proto> m_proto;
	int m_connection_state = -1;
	std::string m_server = "";
	clock_t m_disconnect_time = clock();

	std::mutex m_send_mutex;
	timepoint_t m_sent_tp;

	std::unordered_map<event_type, std::function<void(client *)>, enum_hash> m_events;
	std::unordered_map<hash::hash32_t, std::function<void(client *, networkio::proto::packet *)>> m_endpoints;
	std::unordered_map<hash::hash32_t, std::shared_ptr<base_memory>> m_sharepoints;

	std::mutex m_mutex;
	std::unordered_map<std::string, std::shared_ptr<networkio::memory::base_userdata>> m_userdata;
};

} // namespace mux
} // namespace networkio

#endif
