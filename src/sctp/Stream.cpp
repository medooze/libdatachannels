#include "sctp/Stream.h"
#include "sctp/Association.h"

namespace sctp
{

Stream::Stream(Association &association, uint16_t id) :
	association(association)
{
	this->id = id;
}

Stream::~Stream()
{
}

bool Stream::Recv(std::unique_ptr<sctp::Payload> payload)
{
	if (listener)
	{
		listener->OnMessage(std::move(payload));
	}
	
	return true;
}

bool Stream::Send(std::unique_ptr<sctp::Payload> payload)
{
	payload->streamId = id;

	return association.SendData(std::move(payload));
}

void Stream::SetListener(Listener* listener)
{
	this->listener = listener;
}

}; // namespace sctp
