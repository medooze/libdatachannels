#ifndef DATACHANNEL_IMPL_H_
#define DATACHANNEL_IMPL_H_
#include "datachannels.h"
#include "sctp/Association.h"

namespace datachannels
{
namespace impl
{

class Endpoint : public datachannels::Endpoint
{
public:
	Endpoint(TimeService& timeService,const Options& options);
	virtual ~Endpoint();
	
	// Type definitions
	virtual bool Start(uint16_t remotePort)  override;
	virtual Datachannel::shared CreateDatachannel(const Datachannel::Options& options)  override;
	virtual bool Close()  override;
	
	// Getters
	virtual uint16_t GetLocalPort() const override;
	virtual uint16_t GetRemotePort() const override;
	virtual Setup	 GetSetup() const override;
	virtual datachannels::Transport& GetTransport() override;
private:
	sctp::Association association;
	Setup setup;
};

}; //namespace impl
}; //namespace datachannels
#endif 
