#include "Sctp.h"

namespace datachannels::impl
{
	
size_t Sctp::ReadPacket(uint8_t *data, uint32_t size)
{
	if (endpoints.empty()) return 0;
	
	return endpoints.begin()->second->GetTransport().ReadPacket(data, size);
}

size_t Sctp::WritePacket(uint8_t *data, uint32_t size)
{
	if (endpoints.empty()) return 0;
	
	return endpoints.begin()->second->GetTransport().WritePacket(data, size);
} 


bool Sctp::Close()
{
	std::for_each(endpoints.begin(), endpoints.end(), 
		[](auto& p) { 
			p.second->Close();
		});
		
	return true;
}

std::shared_ptr<Endpoint> Sctp::AddEndpoint(const Endpoint::Options& options)
{
	auto endpoint = std::make_shared<Endpoint>(timeService, listener);
	endpoint->Init(options, mode == Mode::Client);
	
	endpoints[options.ports] = endpoint;
	
	return endpoint;
}

}