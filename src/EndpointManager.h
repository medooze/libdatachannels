#ifndef ENDPOINTMANAGER_H
#define ENDPOINTMANAGER_H

#include "Datachannels.h"
#include "Endpoint.h"

#include <unordered_map>

namespace datachannels
{
namespace impl
{

struct PortsComp
{
	bool operator()(const Ports& lhs, const Ports& rhs) const
	{
		return lhs.localPort == rhs.localPort && lhs.remotePort == rhs.remotePort;
	}
};

struct PortsHash
{
	std::size_t operator()(const Ports& p) const
	{
		return (size_t(p.localPort) << 16) + p.remotePort;
	}
};

class EndpointManager : public datachannels::Transport, public Endpoint::Listener
{
public:
	EndpointManager(TimeService& timeService, 
		datachannels::OnTransportDataPendingListener& onTransportDataPendingListener, 
		datachannels::OnDataChannelCreatedListener& onDataChannelCreatedListener);

	inline Transport& GetTransport() { return *this; }
	void SetEndpointMode(Endpoint::Mode mode);
	datachannels::DataChannel::shared CreateDataChannel(const std::string& label, const std::string& endpointIdentifier = "");
	void Close();
	
	std::vector<std::shared_ptr<datachannels::DataChannel>> GetDataChannels() const;
	
	// datachannels::Transport overrides
	size_t ReadPacket(uint8_t *data, uint32_t size) override;
	size_t WritePacket(uint8_t *data, uint32_t size) override;
	
	// Endpoint::Listener overrides
	virtual void OnAssociationEstablished(const Endpoint::shared& endpoint) override;
	virtual void OnAssociationClosed(const Endpoint::shared& endpoint) override;
	virtual void OnDataChannelCreated(const datachannels::DataChannel::shared& dataChannel) override;
	
	std::optional<std::string> GetEndpointIdentifier(datachannels::DataChannel& dataChannel) const;
	
private:
	std::shared_ptr<Endpoint> AddEndpoint(const Endpoint::Options& options);
	std::optional<Ports> AllocateEndpointPort();

	TimeService& timeService;
	Endpoint::Mode mode = Endpoint::Mode::Server;
	
	std::unordered_map<Ports, std::shared_ptr<Endpoint>, PortsHash, PortsComp> cachedEndpoints;
	std::unordered_map<std::string, std::shared_ptr<Endpoint>> establishedEndpoints;
	std::unordered_map<std::string, std::vector<datachannels::DataChannel>> pendingDataChannelCreation;
	
	datachannels::OnTransportDataPendingListener& onTransportDataPendingListener;
	datachannels::OnDataChannelCreatedListener& onDataChannelCreatedListener;
	
	uint16_t nextPort = 5000;
};

}

}


#endif
