#include "DataChannelFactory.h"

namespace datachannels::impl
{

DataChannelFactory::DataChannelFactory(sctp::Association& association, Endpoint::Mode mode) :
	association(association),
	mode(mode)
{
}

std::shared_ptr<Datachannel> DataChannelFactory::CreateDataChannel()
{
	auto id = allocateStreamId();
	auto channel = std::make_shared<Datachannel>(association, id);
	
	dataChannels[id] = channel;
	
	return channel;
}

const std::map<uint16_t, std::shared_ptr<Datachannel>>& DataChannelFactory::GetDataChannels() const
{
	return dataChannels;
}

void DataChannelFactory::OnStreamCreated(const sctp::Stream::shared& stream)
{
	auto channel = std::make_shared<Datachannel>(stream);
	dataChannels[stream->GetId()] = channel;
}

uint16_t DataChannelFactory::allocateStreamId()
{
	auto maxStreamId = 0;
	if (!dataChannels.empty())
	{
		maxStreamId = (--dataChannels.end())->first;
	}
	
	auto isEven = maxStreamId % 2 == 0;
	if (mode == Endpoint::Mode::Sever)
	{
		// Server mode uses odd number
		return isEven ? (maxStreamId + 1) : (maxStreamId + 2);
	}
	else
	{
		// Client mode uses even number
		return isEven ? (maxStreamId + 2) : (maxStreamId + 1);
	}
}

}