#include "Sctp.h"

using namespace datachannels;

namespace datachannels::impl
{
	
Sctp::Sctp(TimeService& timeService, datachannels::OnDataPendingListener& listener) : 
	timeService(timeService), listener(listener) 
{
};
	
size_t Sctp::ReadPacket(uint8_t *data, uint32_t size)
{
	// @todo Priority chunks are put first. Now it always read the front endpoint
	//       first
	for (auto &endpoint : endpoints)
	{
		auto sz = endpoint.second->GetTransport().ReadPacket(data, size);
		if (sz > 0) return sz;
	}
	
	return 0;
}

size_t Sctp::WritePacket(uint8_t *data, uint32_t size)
{
	BufferReader reader(data, size);
	
	if (size < 4) return 0;
	
	uint16_t sourcePortNumber	= reader.Get2();
	uint16_t destinationPortNumber	= reader.Get2();
	
	Ports ports { sourcePortNumber, destinationPortNumber };
	
	Endpoint::shared endpoint;
	if (endpoints.find(ports) != endpoints.end())
	{
		endpoint = endpoints.at(ports);
	}
	else
	{
		endpoint = AddEndpoint({ports});
	}
	
	return endpoint->GetTransport().WritePacket(data, size);
} 


bool Sctp::Close()
{
	std::for_each(endpoints.begin(), endpoints.end(), 
		[](auto& p) { 
			p.second->Close();
		});
		
	return true;
}

std::shared_ptr<Endpoint> Sctp::AddEndpoint(const Endpoint::Options& options, bool associate)
{
	// @todo Check whether the ports exists already
	
	auto endpoint = std::make_shared<Endpoint>(timeService, listener);
	endpoints[options.ports] = endpoint;
	
	if (!endpoint->Init(options, associate))
	{
		endpoints.erase(options.ports);
		return nullptr;
	}
	
	return endpoint;
}

}