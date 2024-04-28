#ifndef DATACHANNELFACTORY_H
#define DATACHANNELFACTORY_H

#include "sctp/Association.h"
#include "Datachannel.h"

namespace datachannels::impl
{

class DataChannelFactory : public sctp::Association::Listener
{
public:
	DataChannelFactory(sctp::Association& association, Endpoint::Mode mode);
	
	std::shared_ptr<Datachannel> CreateDataChannel();
	
	const std::map<uint16_t, std::shared_ptr<Datachannel>> & GetDataChannels() const;
	
	virtual void OnStreamCreated(const sctp::Stream::shared& stream) override;
	
private:
	
	uint16_t allocateStreamId();

	sctp::Association& association;
	Endpoint::Mode mode;
	
	std::map<uint16_t, std::shared_ptr<Datachannel>> dataChannels;
};	
	
}


#endif