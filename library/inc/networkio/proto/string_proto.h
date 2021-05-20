
#ifndef __PROTO_STRING_PROTO_H__
#define __PROTO_STRING_PROTO_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include <networkio/interfaces/client.h>
#include <networkio/interfaces/proto.h>
#include <networkio/proto/packet.h>
#include <networkio/types.h>

#include <deque>

//----------------------------------------------------------------------------
// namespace
//----------------------------------------------------------------------------

namespace networkio {
namespace proto {

//----------------------------------------------------------------------------
// networkio::proto::string_proto
//----------------------------------------------------------------------------

class string_proto : public virtual interfaces::proto {

  public:
	string_proto(std::shared_ptr<networkio::interfaces::client> client);
	virtual ~string_proto();

  public:
	std::string read_string();
	bool write_string(const std::string &p);

	virtual bool process() override;

  protected:
	std::shared_ptr<networkio::interfaces::client> m_client;

	std::string m_out_string = "";
};

} // namespace proto
} // namespace networkio

#endif
