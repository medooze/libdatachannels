#ifndef DATACHANNEL_IMPL_DATACHANNEL_H_
#define DATACHANNEL_IMPL_DATACHANNEL_H_
#include "Datachannels.h"
#include "Message.h"

#include "sctp/Stream.h"
#include "sctp/Payload.h"

#include <memory>

class BufferReader;
namespace datachannels
{
namespace impl
{
class Datachannel : public datachannels::Datachannel, public sctp::Stream::Listener, public datachannels::MessageProducer, public datachannels::MessageListener
{
public:
	// Message type
	static constexpr uint8_t DATA_CHANNEL_ACK = 0x02;
	static constexpr uint8_t DATA_CHANNEL_OPEN = 0x03;
	
	enum ChannelType
	{
		DATA_CHANNEL_RELIABLE 				= 0x00,
		DATA_CHANNEL_RELIABLE_UNORDERED 		= 0x80,
		DATA_CHANNEL_PARTIAL_RELIABLE_REXMIT 		= 0x01,
		DATA_CHANNEL_PARTIAL_RELIABLE_REXMIT_UNORDERED 	= 0x81,
		DATA_CHANNEL_PARTIAL_RELIABLE_TIMED 		= 0x02,
		DATA_CHANNEL_PARTIAL_RELIABLE_TIMED_UNORDERED	= 0x82
	};

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
	
	bool ParseOpenMessage(BufferReader& reader);
	
	void Dump() const;
	
	State state = State::Unestablished;

	sctp::Stream::shared stream;
	
	// Channel parameters
	ChannelType channelType = DATA_CHANNEL_RELIABLE;
	uint8_t priority = 0;
	uint32_t reliabilityParameters = 0;
	std::string label;
	std::string subprotocol;
};

}; //namespace impl
}; //namespace datachannel
#endif 
