#include "Endpoint.h"
#include <memory>

namespace datachannels
{

namespace impl
{

Endpoint::Endpoint(datachannels::TimeService& timeService, datachannels::OnDataPendingListener& listener) :
	association(timeService, listener),
	factory(association)
{
	
}

Endpoint::~Endpoint()
{
	//Terminate association now!
	association.Abort();
}

bool Endpoint::Init(const Options& options, bool associate)
{
	//Set ports on sctp
	association.SetLocalPort(options.ports.localPort);
	association.SetRemotePort(options.ports.remotePort);
	
	//If we are clients
	if (associate)
		//Start association
		return association.Associate();
	
	//OK, wait for client to associate
	return true;
}

Datachannel::shared Endpoint::CreateDatachannel(const Datachannel::Options& options)
{
	return nullptr;
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
	
}; // namespace impl
}; // namespace datachannel
