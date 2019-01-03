#include "Endpoint.h"
#include <memory>

DatachannelEndpoint::shared DatachannelEndpoint::Create(TimeService& timeService, const Options& options) 
{
	//Create endpoint
	auto endpoint = std::make_shared<datachannel::Endpoint>(timeService,options);
	//Cast and return
	return std::static_pointer_cast<DatachannelEndpoint>(endpoint);
}


namespace datachannel
{

Endpoint::Endpoint(TimeService& timeService, const Options& options) :
	association(timeService),
	setup(options.setup)
{
	
}

Endpoint::~Endpoint()
{
	//Terminate association now!
	association.Abort();
}

bool Endpoint::Start(uint16_t remotePort)
{
	//Set remote port on sctp
	association.SetRemotePort(remotePort);
	
	//If we are clients
	if (setup==Setup::Client)
		//Start association
		return association.Associate();
	
	//OK, wait for client to associate
	return true;
}

Datachannel::shared Endpoint::CreateDatachannel(const Datachannel::Options& options)
{
	
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

Endpoint::Setup Endpoint::GetSetup() const
{
	return setup;
}
	
}; // namespace datachannel
