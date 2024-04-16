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
	
	enum class Mode
	{
		Sever,
		Client	
	};
	
	Sctp(TimeService& timeService) : timeService(timeService) {};

	size_t ReadPacket(uint8_t *data, uint32_t size) override;
	size_t WritePacket(uint8_t *data, uint32_t size) override; 
	
	void OnPendingData(std::function<void(void)> callback) override;
	
	Transport& GetTransport() override
	{
		return *this;
	}
	
	bool Close() override;
	
	std::shared_ptr<Endpoint> AddEndpoint(const Endpoint::Options& options);
	
private:

	Mode mode = Mode::Sever;
	
	TimeService& timeService;
	std::unordered_map<Ports, std::shared_ptr<Endpoint>, PortsHash, PortsComp> endpoints;
	
	std::function<void(void)> onPendingDataCallback;
};

}

}


#endif
