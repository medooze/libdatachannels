#ifndef DATACHANNEL_IMPL_DATACHANNEL_H_
#define DATACHANNEL_IMPL_DATACHANNEL_H_
#include "Datachannels.h"

#include "sctp/Stream.h"
#include "sctp/Payload.h"

#include <memory>

namespace datachannels
{
namespace impl
{
class Datachannel : public datachannels::Datachannel, public sctp::Stream::Listener
{
public:
	Datachannel(const sctp::Stream::shared& stream);
	Datachannel(sctp::Association& association, uint16_t id);
	
	virtual ~Datachannel();
	
	virtual bool Send(MessageType type, const uint8_t* data, const uint64_t size) override;
	virtual bool Close() override;
	
	// Event handlers
	virtual void OnMessage(std::unique_ptr<sctp::Payload> payload) override;
	
private:
	enum class State
	{
		Unestablished,
		Established	
	};
	
	void Open();
	
	State state = State::Unestablished;

	sctp::Stream::shared stream;
};

}; //namespace impl
}; //namespace datachannel
#endif 
