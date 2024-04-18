#ifndef ESTABLISHEDSTATE_H
#define ESTABLISHEDSTATE_H

#include "Events.h"
#include "Fsm.h"
#include "sctp/Chunk.h"
#include "sctp/SequenceNumberWrapper.h"

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
	
	template <typename Event> void onEnter(const Event& event);
	
	fsm::Nothing handle(const ChunkEvent& event);
	fsm::Nothing handle(const ProcessedEvent& event);
	
private:
	void Acknowledge();
	
	Association& association;
	
	TransmissionSequenceNumberWrapper receivedTransmissionSequenceNumberWrapper;
	uint64_t lastReceivedTransmissionSequenceNumber = MaxTransmissionSequenceNumber;
	
	std::multiset<uint64_t> receivedTransmissionSequenceNumbers;

	bool pendingAcknowledge = false;
	std::chrono::milliseconds pendingAcknowledgeTimeout = 0ms;
	
	datachannels::Timer::shared sackTimer;
};

}

#endif
