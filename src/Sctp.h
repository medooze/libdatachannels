#ifndef SCTP_H
#define SCTP_H

#include "Datachannels.h"
#include "Endpoint.h"

#include <unordered_map>

namespace datachannels
{
namespace impl
{

struct Ports
{
	uint16_t localPort = 0;
	uint16_t remotePort = 0;
	
	bool operator==(const Ports& other) const
	{
		return localPort == other.localPort && remotePort == other.remotePort;
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
	
	Sctp(TimeService& timeService) : timeService(timeService) {};

	size_t ReadPacket(uint8_t *data, uint32_t size) override;
	size_t WritePacket(uint8_t *data, uint32_t size) override; 
	
	void OnPendingData(std::function<void(void)> callback) override;
	
	Transport& GetTransport() override
	{
		return *this;
	}
	
	bool Close() override;
	
	std::shared_ptr<Endpoint> AddEndpoint(const Endpoint::Options& options)
	{
		auto endpoint = std::make_shared<Endpoint>(timeService);
		endpoint->Init(options);
		
		Ports ports {options.localPort, options.remotePort};
		endpoints[ports] = endpoint;
		
		return endpoint;
	}
	
private:

	TimeService& timeService;
	std::unordered_map<Ports, std::shared_ptr<Endpoint>, PortsHash> endpoints;
};

}

}


#endif
