#ifndef DATACHANNEL_IMPL_ENDPOINT_H_
#define DATACHANNEL_IMPL_ENDPOINT_H_
#include "Datachannels.h"
#include "sctp/Association.h"
#include "DataChannelFactory.h"

namespace datachannels
{
namespace impl
{

class Endpoint : public datachannels::Endpoint
{
public:
	Endpoint(TimeService& timeService, datachannels::OnTransmissionPendingListener& listener);
	virtual ~Endpoint();
	
	virtual bool Init(const Options& options)  override;
	virtual DataChannel::shared CreateDataChannel(const DataChannel::Options& options)  override;
	virtual bool Close()  override;
	
	// Getters
	virtual uint16_t GetLocalPort() const override;
	virtual uint16_t GetRemotePort() const override;
	virtual datachannels::Transport& GetTransport() override;
	
	inline void SetListener(DataChannelFactory::Listener* listener)
	{
		if (factory != nullptr) factory->SetListener(listener);
	}
	
private:
	sctp::Association association;
	
	std::unique_ptr<DataChannelFactory> factory;
};

}; //namespace impl
}; //namespace datachannels
#endif 
