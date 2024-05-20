#include "EndpointManager.h"

#include <limits>

using namespace datachannels;

namespace datachannels::impl
{
	
EndpointManager::EndpointManager(TimeService& timeService, datachannels::OnTransportDataPendingListener& onTransportDataPendingListener, 
		datachannels::OnDataChannelCreatedListener& onDataChannelCreatedListener) : 
	timeService(timeService), 
	onTransportDataPendingListener(onTransportDataPendingListener),
	onDataChannelCreatedListener(onDataChannelCreatedListener)
{
};

void EndpointManager::SetEndpointMode(Endpoint::Mode mode)
{
	this->mode = mode;
}

void EndpointManager::CreateDataChannel(const std::string& label, const std::string& endpointIdentifier)
{
	if (establishedEndpoints.find(endpointIdentifier) != establishedEndpoints.end())
	{
		establishedEndpoints[endpointIdentifier]->CreateDataChannel({label});
	}
	else
	{
		if (mode == Endpoint::Mode::Client)
		{
			auto ports = AllocateEndpointPort();
			if (ports)
			{
				auto endpoint = AddEndpoint({*ports, mode});
				endpoint->Connect();
			}
			else
			{
				Error("Failed to find available sctp endpoing ports\n");
				return;
			}
		}
		
		pendingDataChannelCreation.push_back({label, endpointIdentifier});
	}
}


void EndpointManager::Close()
{
	std::for_each(cachedEndpoints.begin(), cachedEndpoints.end(), 
		[](auto& p) { 
			p.second->Close();
		});
}

std::vector<std::shared_ptr<datachannels::DataChannel>> EndpointManager::GetDataChannels() const
{
	std::vector<std::shared_ptr<datachannels::DataChannel>> dataChannels;
	
	std::for_each(establishedEndpoints.begin(), establishedEndpoints.end(), 
		[&dataChannels](auto& p) { 
			auto dcs = p.second->GetDataChannels();
			
			std::for_each(dcs.begin(), dcs.end(), [&dataChannels](auto& p)
			{
				if (p.second->IsOpen()) dataChannels.push_back(p.second);
			});
			
		});
	
	return dataChannels;
}

size_t EndpointManager::ReadPacket(uint8_t *data, uint32_t size)
{
	// @todo Priority chunks are put first. Now it always read the front endpoint
	//       first
	for (auto &endpoint : cachedEndpoints)
	{
		auto sz = endpoint.second->GetTransport().ReadPacket(data, size);
		if (sz > 0) return sz;
	}
	
	return 0;
}

size_t EndpointManager::WritePacket(uint8_t *data, uint32_t size)
{
	BufferReader reader(data, size);
	
	if (size < 4) return 0;
	
	uint16_t sourcePortNumber	= reader.Get2();
	uint16_t destinationPortNumber	= reader.Get2();
	
	Ports ports { sourcePortNumber, destinationPortNumber };
	
	Endpoint::shared endpoint;
	if (cachedEndpoints.find(ports) != cachedEndpoints.end())
	{
		endpoint = cachedEndpoints.at(ports);
	}
	else
	{
		// Request from client, current endpoint is server mode
		endpoint = AddEndpoint({ports, mode});
	}
	
	return endpoint->GetTransport().WritePacket(data, size);
}

void EndpointManager::OnAssociationEstablished(const Endpoint::shared& endpoint)
{
	establishedEndpoints[endpoint->GetIdentifier()] = endpoint;
	
	for (auto it = pendingDataChannelCreation.begin(); it != pendingDataChannelCreation.end();)
	{
		if (it->second == endpoint->GetIdentifier())
		{
			endpoint->CreateDataChannel({it->first});
			it = pendingDataChannelCreation.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void EndpointManager::OnAssociationClosed(const Endpoint::shared& endpoint)
{
	establishedEndpoints.erase(endpoint->GetIdentifier());
	cachedEndpoints.erase(Ports{endpoint->GetLocalPort(), endpoint->GetRemotePort()});
}

void EndpointManager::OnDataChannelCreated(const datachannels::DataChannel::shared& dataChannel)
{
	onDataChannelCreatedListener.OnDataChannelCreated(dataChannel);
}

std::optional<std::string> EndpointManager::GetEndpointIdentifier(datachannels::DataChannel& dataChannel) const
{
	auto ports = dataChannel.GetPorts();
	
	if (cachedEndpoints.find(ports) != cachedEndpoints.end())
		return cachedEndpoints.at(ports)->GetIdentifier();
		
	return std::nullopt;
}

std::shared_ptr<Endpoint> EndpointManager::AddEndpoint(const Endpoint::Options& options)
{
	auto endpoint = std::make_shared<Endpoint>(timeService, onTransportDataPendingListener);
	cachedEndpoints[options.ports] = endpoint;
	
	if (!endpoint->Init(options))
	{
		cachedEndpoints.erase(options.ports);
		return nullptr;
	}
	
	endpoint->SetListener(this);
	
	return endpoint;
}

std::optional<Ports> EndpointManager::AllocateEndpointPort()
{
	if (mode == Endpoint::Mode::Server) return std::nullopt;
	
	for (uint16_t i = 0; i < std::numeric_limits<uint16_t>::max(); i++)
	{
		Ports ports {nextPort, nextPort};
		
		if (cachedEndpoints.find(ports) == cachedEndpoints.end())
		{
			return ports;
		}
		
		nextPort++;
	}
	
	return std::nullopt;
}

}