#ifndef DATACHANNELFACTORY_H
#define DATACHANNELFACTORY_H

#include <sctp/Association.h>
#include <Datachannel.h>

namespace datachannels::impl
{

class DataChannelFactory : public sctp::Association::Listener
{
public:
	DataChannelFactory(sctp::Association& association);
	
	std::shared_ptr<Datachannel> CreateDataChannel();
	
	const std::vector<std::shared_ptr<Datachannel>> & GetDataChannels() const;
	
	virtual void OnStreamCreated(const sctp::Stream::shared& stream) override;
	
private:
	sctp::Association& association;
	
	std::vector<std::shared_ptr<Datachannel>> dataChannels;
};	
	
}


#endif