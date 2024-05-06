#ifndef SCTP_H
#define SCTP_H

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

class Sctp : public datachannels::Sctp, public datachannels::Transport
{
public:
	class Listener
	{
	public:
		virtual ~Listener() = default;
		virtual void OnDataChannelOpen(const std::string& endpointId, const std::shared_ptr<datachannels::DataChannel>& dataChannel) = 0;
	};
	
	Sctp(TimeService& timeService, datachannels::OnTransmissionPendingListener& listener);

	size_t ReadPacket(uint8_t *data, uint32_t size) override;
	size_t WritePacket(uint8_t *data, uint32_t size) override; 
	
	inline Transport& GetTransport() override	{ return *this; }
	bool Close() override;
	
	void CreateDataChannel(const std::string& label, const std::string& endpointIdentifier = "");
	
	void SetListener(const std::shared_ptr<Listener>& listener);
	
private:
	std::shared_ptr<Endpoint> AddEndpoint(const Endpoint::Options& options);

	Endpoint::Mode mode = Endpoint::Mode::Sever;
	
	TimeService& timeService;
	std::unordered_map<Ports, std::shared_ptr<Endpoint>, PortsHash, PortsComp> allEndpoints;
	
	std::unordered_map<std::string, std::shared_ptr<Endpoint>> endpoints;
	
	datachannels::OnTransmissionPendingListener& listener;
	
	std::shared_ptr<Listener> dclistener;
};

}

}


#endif
