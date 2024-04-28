#ifndef DATACHANNEL_IMPL_DATACHANNEL_H_
#define DATACHANNEL_IMPL_DATACHANNEL_H_
#include "Datachannels.h"
#include "Message.h"

#include "sctp/Stream.h"
#include "sctp/Payload.h"

#include <memory>

namespace datachannels
{
namespace impl
{
class Datachannel : public datachannels::Datachannel, public sctp::Stream::Listener, public datachannels::MessageProducer, public datachannels::MessageListener
{
public:
	Datachannel(const sctp::Stream::shared& stream);
	Datachannel(sctp::Association& association, uint16_t id);
	
	virtual ~Datachannel();
	
	virtual bool Send(datachannels::MessageType type, const uint8_t* data, const uint64_t size) override;
	virtual bool Close() override;
	
	// sctp::Stream::Listener
	virtual void OnPayload(std::unique_ptr<sctp::Payload> payload) override;
	
	// Listener
	virtual void OnMessage(const datachannels::Message& message) {};
	virtual void AddMessageListener(const std::shared_ptr<MessageListener>& listener) {};
	virtual void RemoveMessageListener(const std::shared_ptr<MessageListener>& listener) {};
	
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
