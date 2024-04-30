#include "Endpoint.h"
#include <memory>

namespace datachannels
{

namespace impl
{

Endpoint::Endpoint(datachannels::TimeService& timeService, datachannels::OnTransmissionPendingListener& listener) :
	association(timeService, listener)
{
	
}

Endpoint::~Endpoint()
{
	//Terminate association now!
	association.Abort();
}

bool Endpoint::Init(const Options& options)
{
	factory = std::make_unique<DataChannelFactory>(association, mode);
	
	//Set ports on sctp
	association.SetLocalPort(options.ports.localPort);
	association.SetRemotePort(options.ports.remotePort);
	
	//If we are clients
	if (options.mode == Mode::Client)
		//Start association
		return association.Associate();
	
	//OK, wait for client to associate
	return true;
}

DataChannel::shared Endpoint::CreateDataChannel(const DataChannel::Options& options)
{
	if (!factory) return nullptr;
	
	return factory->CreateDataChannel();
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
