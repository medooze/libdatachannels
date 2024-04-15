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

}