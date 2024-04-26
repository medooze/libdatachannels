#include "Datachannel.h"

namespace datachannels
{
namespace impl
{

Datachannel::Datachannel(const sctp::Stream::shared& stream)
{
	this->stream = stream;
	stream->SetListener(this);
}

Datachannel::Datachannel(sctp::Association& association, uint16_t id)
{
	this->stream = std::make_shared<sctp::Stream>(association, id);
	stream->SetListener(this);
	
	Open();
}


Datachannel::~Datachannel()
{
	stream->SetListener(nullptr);
}
	
bool Datachannel::Send(MessageType type, const uint8_t* data, const uint64_t size)
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
		payload->type = type==UTF8 ? sctp::PayloadType::WebRTCStringEmpty : sctp::PayloadType::WebRTCBinaryEmpty;
	}
	else 
	{
		payload->type = type==UTF8 ? sctp::PayloadType::WebRTCString : sctp::PayloadType::WebRTCBinary;
		payload->data = Buffer(data, size);
	}
	
	return stream->Send(std::move(payload));
}

bool Datachannel::Close()
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

void Datachannel::OnMessage(std::unique_ptr<sctp::Payload> payload)
{
	if (state == State::Unestablished)
	{
		// @todo Hanlde DECP messsages
	}
	else if (state == State::Established)
	{
		
	}
	else
	{
		Error("Unexpected state\n");
	}
}

void Datachannel::Open()
{
	// @todo DCEP
}

}; //namespace impl
}; //namespace datachannel
