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
class DataChannel : public datachannels::DataChannel, 
		public sctp::Stream::Listener, 
		public datachannels::MessageProducer, 
		public datachannels::MessageListener,
		public std::enable_shared_from_this<DataChannel>
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
	
	class Listener
	{
	public:
		virtual ~Listener() = default;
		virtual void OnOpen(const datachannels::DataChannel::shared& dataChannel) = 0;
		virtual void OnClosed(const datachannels::DataChannel::shared& dataChannel) = 0;
	};
	
	DataChannel(const sctp::Stream::shared& stream);
	DataChannel(sctp::Association& association, uint16_t id);
	
	virtual ~DataChannel();
	
	virtual bool Send(datachannels::MessageType type, const uint8_t* data, const uint64_t size) override;
	virtual bool Close() override;
	
	// sctp::Stream::Listener
	virtual void OnPayload(std::shared_ptr<sctp::Payload> payload) override;
	
	// Listener
	virtual void OnMessage(const std::shared_ptr<Message>& message) override;
	
	virtual void AddMessageListener(const std::shared_ptr<MessageListener>& listener) override;
	virtual void RemoveMessageListener(const std::shared_ptr<MessageListener>& listener) override;
	
	void Open();
	
	void SetListener(Listener* listener);
private:
	enum class State
	{
		Unestablished,
		Established	
	};
	
	
	bool ParseOpenMessage(BufferReader& reader);
	
	void Dump() const;
	
	State state = State::Unestablished;

	sctp::Stream::shared stream;
	Listener* listener = nullptr;
	
	datachannels::TimeService& timeService;
	
	// Channel parameters
	ChannelType channelType = DATA_CHANNEL_RELIABLE;
	uint8_t priority = 0;
	uint32_t reliabilityParameters = 0;
	std::string label;
	std::string subprotocol;
	
	std::list<std::shared_ptr<MessageListener>> messageListeners;
};

}; //namespace impl
}; //namespace datachannel
#endif 
