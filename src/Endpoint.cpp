#include "Endpoint.h"
#include <memory>

namespace datachannels
{

namespace impl
{

Endpoint::Endpoint(datachannels::TimeService& timeService, datachannels::OnTransportDataPendingListener& listener) :
	association(timeService, *this, listener)
{
}

Endpoint::~Endpoint()
{
	//Terminate association now!
	association.Abort();
	
	for (auto& [id, channel] : dataChannels)
	{
		channel->SetListener(nullptr);
	}
}

bool Endpoint::Init(const Options& options)
{
	mode = options.mode;
	
	//Set ports on sctp
	association.SetLocalPort(options.ports.localPort);
	association.SetRemotePort(options.ports.remotePort);
	
	//OK, wait for client to associate
	return true;
}

void Endpoint::Connect()
{
	association.Associate();
}

DataChannel::shared Endpoint::CreateDataChannel(const DataChannel::Options& options)
{
	auto id = allocateStreamId();
	auto channel = std::make_shared<DataChannel>(association, id);
	
	dataChannels[id] = channel;
	
	return channel;
}

bool Endpoint::Close()
{
	//Gracefuly stop association
	return association.Shutdown();
}

// Getters
uint16_t Endpoint::GetLocalPort() const
{
	return association.GetLocalPort();
}

uint16_t Endpoint::GetRemotePort() const
{
	return association.GetRemotePort();
}

datachannels::Transport& Endpoint::GetTransport()
{
	return association;
}

const std::map<uint16_t, std::shared_ptr<DataChannel>>& Endpoint::GetDataChannels() const
{
	return dataChannels;
}

void Endpoint::OnStreamCreated(const sctp::Stream::shared& stream)
{
	Debug("Endpoint::OnStreamCreated: %d\n", stream->GetId());
	
	auto channel = std::make_shared<DataChannel>(stream);
	dataChannels[stream->GetId()] = channel;
	
	channel->SetListener(this);
}

void Endpoint::OnEstablished(sctp::Association* association)
{
	if (listener != nullptr) listener->OnAssociationEstablished(shared_from_this());
}

void Endpoint::OnClosed(sctp::Association* association)
{
	if (listener != nullptr) listener->OnAssociationClosed(shared_from_this());
}

void Endpoint::OnOpen(const datachannels::DataChannel::shared& dataChannel)
{
	if (listener != nullptr) listener->OnDataChannelOpen(identifier, dataChannel);
}

void Endpoint::OnClosed(const datachannels::DataChannel::shared& dataChannel)
{
	if (listener != nullptr) listener->OnDataChannelClose(identifier, dataChannel);
}

uint16_t Endpoint::allocateStreamId()
{
	auto maxStreamId = 0;
	if (!dataChannels.empty())
	{
		maxStreamId = (--dataChannels.end())->first;
	}
	
	auto isEven = maxStreamId % 2 == 0;
	if (mode == Endpoint::Mode::Server)
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

	
}; // namespace impl
}; // namespace datachannel
