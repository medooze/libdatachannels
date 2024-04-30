#include "DataChannelFactory.h"

namespace datachannels::impl
{

DataChannelFactory::DataChannelFactory(sctp::Association& association, Endpoint::Mode mode) :
	association(association),
	mode(mode)
{
	association.SetListener(this);
}

std::shared_ptr<DataChannel> DataChannelFactory::CreateDataChannel()
{
	auto id = allocateStreamId();
	auto channel = std::make_shared<DataChannel>(association, id);
	
	dataChannels[id] = channel;
	
	return channel;
}

const std::map<uint16_t, std::shared_ptr<DataChannel>>& DataChannelFactory::GetDatachannels() const
{
	return dataChannels;
}

void DataChannelFactory::OnStreamCreated(const sctp::Stream::shared& stream)
{
	Debug("DataChannelFactory::OnStreamCreated: %d\n", stream->GetId());
	
	auto channel = std::make_shared<DataChannel>(stream);
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