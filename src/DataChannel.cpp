#include "DataChannel.h"
#include "internal/BufferReader.h"
#include "sctp/Association.h"

namespace datachannels
{
namespace impl
{

DataChannel::DataChannel(const sctp::Stream::shared& stream) :
	timeService(stream->getAssociation().GetTimeService())
{
	this->stream = stream;
	
	stream->SetListener(this);
}

DataChannel::DataChannel(sctp::Association& association, uint16_t id) :
	timeService(association.GetTimeService())
{
	this->stream = association.createStream(id);
	
	stream->SetListener(this);
}

DataChannel::~DataChannel()
{
	stream->SetListener(nullptr);
}
	
bool DataChannel::Send(datachannels::MessageType type, const uint8_t* data, const uint64_t size)
{
	auto payload = std::make_shared<datachannels::Message>();
	
	payload->type = type;
	payload->data.assign(data, data + size);
	
	OnMessage(payload);
	
	return true;
}

bool DataChannel::Close()
{
	//   Closing of a data channel MUST be signaled by resetting the
	//   corresponding outgoing streams [RFC6525].  This means that if one
	//   side decides to close the data channel, it resets the corresponding
	//   outgoing stream.  When the peer sees that an incoming stream was
	//   reset, it also resets its corresponding outgoing stream.  Once this
	//   is completed, the data channel is closed.  Resetting a stream sets
	//   the Stream Sequence Numbers (SSNs) of the stream back to 'zero' with
	//   a corresponding notification to the application layer that the reset
	//   has been performed.  Streams are available for reuse after a reset
	//   has been performed.
	//
	//   [RFC6525] also guarantees that all the messages are delivered (or
	//   abandoned) before the stream is reset.	
	return true;
}

std::string DataChannel::GetLabel() const
{
	return label;
}

void DataChannel::OnPayload(std::shared_ptr<datachannels::Message> payload)
{
	Debug("DataChannel::OnPayload\n");
		
	BufferReader reader(payload->data.data(), payload->data.size());
	
	if (state == State::Unestablished)
	{
		if (payload->type == datachannels::MessageType::DCEP)
		{
			if (ParseOpenMessage(reader))
			{
				std::shared_ptr<datachannels::Message> ack = std::make_unique<datachannels::Message>();
				ack->type = datachannels::MessageType::DCEP;
				
				uint8_t ackType = DATA_CHANNEL_ACK;
				ack->data.resize(1);
				ack->data[0] = ackType;
				
				Dump();
				
				stream->Send(ack);
				
				state = State::Established;
				
				if (listener) listener->OnOpen(shared_from_this());
			}
			else
			{
				Debug("Failed to parse open message.");
			}
		}
	}
	else if (state == State::Established)
	{
		if (payload->type == MessageType::WebRTCString)
		{
			std::string str;
			str.assign((char*)payload->data.data(), payload->data.size());
			
			Debug("-Data message: %s\n", str.c_str());
		}
		
		for (auto& listener : messageListeners)
		{
			listener->OnMessage(payload);
		}
	}
	else
	{
		Error("Unexpected state\n");
	}
}

void DataChannel::Open()
{
	// @todo DCEP
}

void DataChannel::OnMessage(const std::shared_ptr<datachannels::Message>& payload)
{
	std::weak_ptr<DataChannel> weak = shared_from_this();
	timeService.Async([weak, payload](...) {
		auto self = weak.lock();
		if (!self) return;
		
		if (self->state != State::Established)
		{
			Error("Data channel is not established. Skip sending.\n");
			return;
		}

		if (!self->stream->Send(payload))
		{
			Error("Faild to send stream data. Skip sending.\n");
			return;
		}
	});
}

void DataChannel::AddMessageListener(const std::shared_ptr<MessageListener>& listener)
{
	std::weak_ptr<DataChannel> weak = shared_from_this();
	timeService.Async([weak, listener](...){
		auto self = weak.lock();
		if (!self) return;
		
		self->messageListeners.push_back(listener);
	});
}

void DataChannel::RemoveMessageListener(const std::shared_ptr<MessageListener>& listener)
{
	std::weak_ptr<DataChannel> weak = shared_from_this();
	timeService.Async([weak, listener](...){
		auto self = weak.lock();
		if (!self) return;
		
		self->messageListeners.remove(listener);
	});
}

void DataChannel::SetListener(datachannels::DataChannel::Listener* listener)
{
	timeService.Sync([this, listener](...) {
		this->listener = listener;	
	});
}

bool DataChannel::ParseOpenMessage(BufferReader& reader)
{
	if (reader.GetSize() < 1)
	{
		Debug("Invalid DCEP message\n");
		return false;
	}
	
	auto msgType = reader.Get1();
	if (msgType != DATA_CHANNEL_OPEN)
	{
		Debug("Message type is not DATA_CHANNEL_OPEN, 0x%x\n", msgType);
		return false;
	}
	
	if (reader.GetLeft() < 11)
	{
		Debug("Invalid DATA_CHANNEL_OPEN message size.\n");
		return false;
	}
	
	auto chnType = reader.Get1();
	auto prio= reader.Get2();
	auto reliability = reader.Get4();
	
	auto labelLen = reader.Get2();
	auto protocolLen = reader.Get2();
	
	if (reader.GetLeft() < (labelLen + protocolLen))
	{
		Debug("Invalid DATA_CHANNEL_OPEN message size.\n");
		return false;
	}
	
	// Set members
	channelType = ChannelType(chnType);
	priority = prio;
	reliabilityParameters = reliability;
	
	label.assign(reinterpret_cast<const char *>(reader.GetData(labelLen)), labelLen);
	subprotocol.assign(reinterpret_cast<const char *>(reader.GetData(protocolLen)), protocolLen);
	
	return true;
}

uint16_t DataChannel::GetId() const
{
	return stream->GetId();
}

void DataChannel::Dump() const
{
	Debug("Data channel Info:\n");
	Debug("channelType: 0x%x\n", channelType);
	Debug("priority: %d\n", int(priority));
	Debug("reliabilityParameters:: %d\n", reliabilityParameters);
	Debug("label: %s\n", label.c_str());
	Debug("subprotocol: %s\n", subprotocol.c_str());
}
}; //namespace impl
}; //namespace datachannel
