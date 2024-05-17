#ifndef DATACHANNEL_IMPL_ENDPOINT_H_
#define DATACHANNEL_IMPL_ENDPOINT_H_
#include "Datachannels.h"
#include "DataChannel.h"
#include "sctp/Association.h"

namespace datachannels
{
namespace impl
{

class Endpoint : public datachannels::Endpoint,
		public sctp::Association::Listener,
		public std::enable_shared_from_this<Endpoint>
{
public:
	using shared = std::shared_ptr<Endpoint>;
	
	class Listener
	{
	public:
		virtual void OnAssociationEstablished(const Endpoint::shared& endpoint) = 0;
		virtual void OnAssociationClosed(const Endpoint::shared& endpoint) = 0;
		
		virtual void OnDataChannelCreated(const datachannels::DataChannel::shared& dataChannel) = 0;
	};

	Endpoint(TimeService& timeService, datachannels::OnTransportDataPendingListener& dataPendingListener);
	virtual ~Endpoint();
	
	virtual bool Init(const Options& options)  override;
	virtual DataChannel::shared CreateDataChannel(const DataChannel::Options& options)  override;
	virtual bool Close()  override;
	
	// Getters
	virtual uint16_t GetLocalPort() const override;
	virtual uint16_t GetRemotePort() const override;
	virtual datachannels::Transport& GetTransport() override;
	
	void Connect();
	
	const std::map<uint16_t, std::shared_ptr<DataChannel>> & GetDataChannels() const;

	// sctp::Association::Listener overrides
	void OnStreamCreated(const sctp::Stream::shared& stream) override;
	void OnEstablished(sctp::Association* association) override;
	void OnClosed(sctp::Association* association) override;
	
	inline void SetListener(Listener* listener)
	{
		this->listener = listener;
	}
	
	inline std::string GetIdentifier() const
	{
		return identifier;
	}
	
private:
	uint16_t allocateStreamId();
	
	datachannels::Endpoint::Mode mode = datachannels::Endpoint::Mode::Server;
	sctp::Association association;
	std::string identifier;
	
	std::map<uint16_t, std::shared_ptr<DataChannel>> dataChannels;
	
	Listener* listener = nullptr;
};

}; //namespace impl
}; //namespace datachannels
#endif 
