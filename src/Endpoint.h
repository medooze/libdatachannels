#ifndef DATACHANNEL_IMPL_H_
#define DATACHANNEL_IMPL_H_
#include "datachannels.h"
#include "sctp/Association.h"

namespace datachannel
{

class Endpoint : public DatachannelEndpoint
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
	
	
	//DTLS tranport
	virtual size_t ReadPacket(uint8_t *data, uint32_t size) override;
	virtual size_t WritePacket(uint8_t *data, uint32_t size) override; 
	
	virtual void OnPendingData(std::function<void(void)> callback) override;
	
private:
	sctp::Association association;
	Setup setup;
};
	
};
#endif 
