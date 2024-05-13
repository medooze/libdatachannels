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

bool Stream::Recv(std::shared_ptr<datachannels::Message> payload)
{
	if (listener)
	{
		listener->OnPayload(std::move(payload));
	}
	
	return true;
}

bool Stream::Send(std::shared_ptr<datachannels::Message> payload)
{
	return association.SendData(id, std::move(payload));
}

void Stream::SetListener(Listener* listener)
{
	this->listener = listener;
}

}; // namespace sctp
