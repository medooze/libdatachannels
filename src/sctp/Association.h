#ifndef SCTP_ASSOCIATION_H_
#define SCTP_ASSOCIATION_H_
#include <list>
#include <map>
#include <set>

#include "Datachannels.h"
#include "sctp/DataReceiver.h"
#include "sctp/Transmitter.h"
#include "sctp/SequenceNumberWrapper.h"
#include "sctp/PacketHeader.h"
#include "sctp/Stream.h"
#include "BufferWritter.h"
#include "BufferReader.h"
#include "states/ClosedState.h"
#include "states/CookieWaitState.h"
#include "states/CookieEchoedState.h"
#include "states/EstablishedState.h"

using namespace std::chrono_literals;

namespace sctp
{

using AssociationFsm = fsm::StateMachine<ClosedState, CookieWaitState, CookieEchoedState, EstablishedState>;

class Association : public datachannels::Transport, public DataReceiver::Listener, public Transmitter
{
public:
	class Listener
	{
	public:
		virtual void OnStreamCreated(const sctp::Stream::shared& stream) = 0;

		virtual void OnEstablished(Association* association) = 0;
		virtual void OnClosed(Association* association) = 0;
	};

	Association(datachannels::TimeService& timeService, datachannels::OnTransportDataPendingListener& dataPendingListener);
	virtual ~Association();
	
	bool Associate();
	bool Shutdown();
	bool Abort();

	void SetLocalPort(uint16_t port) 	{ localPort = port;	}
	void SetRemotePort(uint16_t port) 	{ remotePort = port;	}
	uint16_t GetLocalPort() const 		{ return localPort;	}
	uint16_t GetRemotePort() const		{ return remotePort;	}
	bool HasPendingData() const		{ return pendingData;	}
	
	void SetLocalVerificationTag(uint32_t tag) { localVerificationTag = tag; }
	void SetRemoteVerificationTag(uint32_t tag) { remoteVerificationTag = tag; }
	
	void NotifyEstablished()
	{
		if (this->listener != nullptr) this->listener->OnEstablished(this);
	}
	
	void NotifyClosed()
	{
		if (this->listener != nullptr) this->listener->OnClosed(this);
	}
	
	inline void SetListener(Listener* listener)
	{
		this->listener = listener;
	}
	
	inline datachannels::TimeService& GetTimeService()
	{
		return timeService;
	}
	
	// Interfaces to transport
	
	virtual size_t ReadPacket(uint8_t *data, uint32_t size) override;
	virtual size_t WritePacket(uint8_t *data, uint32_t size) override;
		
	inline size_t ReadPacket(Buffer& buffer)
	{
		size_t len = ReadPacket(buffer.GetData(),buffer.GetCapacity());
		buffer.SetSize(len);
		return len;
	}
	inline size_t WritePacket(Buffer& buffer)
	{
		return WritePacket(buffer.GetData(),buffer.GetSize());
	}

	template <typename Event>
	void HandleEvent(const Event& event)
	{
		fsm.handle(event);
	}

	// Interfaces for chunk
	
	void Enqueue(const Chunk::shared& chunk) override;
	void Enqueue(const std::vector<Chunk::shared>& chunkBundle) override;
	
	// Upper level interfaces
	
	void OnDataReceived(uint16_t streamId, std::unique_ptr<sctp::Payload> data) override;
	
	bool SendData(uint16_t streamId, std::unique_ptr<sctp::Payload> data);
	
	Stream::shared createStream(uint16_t streamId);
	
private:

	std::list<Chunk::shared> queue;
	datachannels::TimeService& timeService;
	
	uint16_t localPort = 0;
	uint16_t remotePort = 0;
	uint32_t localVerificationTag = 0;
	uint32_t remoteVerificationTag = 0;
	
	bool pendingData = false;

	std::map<uint16_t,Stream::shared> streams;
	
	datachannels::OnTransportDataPendingListener& dataPendingListener;
	
	Listener* listener = nullptr;
	
	ClosedState closedState;
	CookieWaitState cookieWaitState;
	CookieEchoedState cookieEchoedState;
	EstablishedState establishedState;
	
	AssociationFsm fsm;
};

}
#endif
