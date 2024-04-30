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
	
	Open();
}


DataChannel::~DataChannel()
{
	stream->SetListener(nullptr);
}
	
bool DataChannel::Send(datachannels::MessageType type, const uint8_t* data, const uint64_t size)
{
	if (state != State::Established) return false;
	
	auto payload = std::make_unique<sctp::Payload>();
	
	if (!data || !size)
	{
		//   SCTP does not support the sending of empty user messages.  Therefore,
		//   if an empty message has to be sent, the appropriate PPID (WebRTC
		//   String Empty or WebRTC Binary Empty) is used and the SCTP user
		//   message of one zero byte is sent.  When receiving an SCTP user
		//   message with one of these PPIDs, the receiver MUST ignore the SCTP
		//   user message and process it as an empty message.
		payload->type = type==datachannels::MessageType::UTF8 ? sctp::PayloadType::WebRTCStringEmpty : sctp::PayloadType::WebRTCBinaryEmpty;
	}
	else 
	{
		payload->type = type==datachannels::MessageType::UTF8 ? sctp::PayloadType::WebRTCString : sctp::PayloadType::WebRTCBinary;
		payload->data = Buffer(data, size);
	}
	
	return stream->Send(std::move(payload));
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

void DataChannel::OnPayload(std::unique_ptr<sctp::Payload> payload)
{
	Debug("DataChannel::OnPayload\n");
		
	BufferReader reader(payload->data);
	
	if (state == State::Unestablished)
	{
		if (payload->type == sctp::PayloadType::DCEP)
		{
			if (ParseOpenMessage(reader))
			{
				std::unique_ptr<sctp::Payload> ack = std::make_unique<sctp::Payload>();
				ack->type = sctp::PayloadType::DCEP;
				
				uint8_t ackType = DATA_CHANNEL_ACK;
				ack->data = Buffer(&ackType, 1);
				
				Dump();
				
				stream->Send(std::move(ack));
				
				state = State::Established;
			}
			else
			{
				Debug("Failed to parse open message.");
			}
		}
	}
	else if (state == State::Established)
	{
		auto type = datachannels::MessageType::UTF8;
		
		switch (payload->type)
		{
		case sctp::PayloadType::WebRTCString:
		case sctp::PayloadType::WebRTCStringEmpty:
			type = datachannels::MessageType::UTF8;
			break;
		case sctp::PayloadType::WebRTCBinary:
		case sctp::PayloadType::WebRTCBinaryEmpty:
			type = datachannels::MessageType::UTF8;
			break;
		default:
			Debug("Unexpected payload type: %d\n", type);
			return;
		}
		
		auto message = std::make_shared<datachannels::Message>(type, stream->getAssociation().GetLocalPort(), stream->GetId());
		for (auto& listener : messageListeners)
		{
			listener->OnMessage(message);
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

void DataChannel::OnMessage(const std::shared_ptr<datachannels::Message>& message)
{
	std::weak_ptr<DataChannel> weak = shared_from_this();
	timeService.Async([weak, message](...){
		auto self = weak.lock();
		if (!self) return;
		
		self->Send(message->GetInfo().type, message->GetData(), message->GetLength());
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
