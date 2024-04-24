#ifndef ESTABLISHEDSTATE_H
#define ESTABLISHEDSTATE_H

#include "Events.h"
#include "Fsm.h"
#include "sctp/Chunk.h"
#include "sctp/SequenceNumberWrapper.h"
#include "sctp/DataReceiver.h"
#include "sctp/DataSender.h"

namespace sctp
{

class Association;

class EstablishedState : public fsm::ByDefault<fsm::Nothing>
{
public:
	using ByDefault::handle;
	using TransmissionSequenceNumberWrapper = SequenceNumberWrapper<uint32_t>;
	
	static constexpr uint64_t MaxTransmissionSequenceNumber = TransmissionSequenceNumberWrapper::MaxSequenceNumber;
	static constexpr std::chrono::milliseconds SackTimeout = 100ms;

	EstablishedState(Association& association);
	
	template <typename Event> void onEnter(const Event& event, const std::pair<uint32_t, uint32_t>& parameters);
	template <typename Event> void onLeave(const Event& event);
	
	fsm::Nothing handle(const ChunkEvent& event);
	fsm::Nothing handle(const ProcessedEvent& event);
	
private:
	void Acknowledge();
	
	Association& association;
	
	std::shared_ptr<DataReceiver> dataReceiver;
	std::shared_ptr<DataSender> dataSender;

	bool pendingAcknowledge = false;
	std::chrono::milliseconds pendingAcknowledgeTimeout = 0ms;
	
	datachannels::Timer::shared sackTimer;
};

}

#endif
