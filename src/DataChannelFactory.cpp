#include "DataChannelFactory.h"

namespace datachannels::impl
{

std::shared_ptr<Datachannel> DataChannelFactory::CreateDataChannel()
{
	return nullptr;
}

const std::vector<std::shared_ptr<Datachannel>> & DataChannelFactory::GetDataChannels() const
{
	return dataChannels;
}

void DataChannelFactory::OnStreamCreated(const sctp::Stream::shared& stream)
{
	auto channel = std::make_shared<Datachannel>(stream);
	dataChannels.push_back(channel);
}

}