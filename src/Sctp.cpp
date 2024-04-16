#include "Sctp.h"

namespace datachannels::impl
{
	
size_t Sctp::ReadPacket(uint8_t *data, uint32_t size)
{
	return 0;
}

size_t Sctp::WritePacket(uint8_t *data, uint32_t size)
{
	return 0;
} 

void Sctp::OnPendingData(std::function<void(void)> callback)
{
	
}

bool Sctp::Close()
{
	return true;
}

std::shared_ptr<Endpoint> Sctp::AddEndpoint(const Endpoint::Options& options)
{
	auto endpoint = std::make_shared<Endpoint>(timeService);
	endpoint->Init(options, mode == Mode::Client);
	
	endpoints[options.ports] = endpoint;
	
	return endpoint;
}

}