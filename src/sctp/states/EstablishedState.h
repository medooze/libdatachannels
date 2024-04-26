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

	EstablishedState(Association& association);
	
	template <typename Event> void onEnter(const Event& event, const std::pair<uint32_t, uint32_t>& parameters);
	template <typename Event> void onLeave(const Event& event);
	
	fsm::Nothing handle(const ChunkEvent& event);
	fsm::Nothing handle(const PacketProcessedEvent& event);
	
private:
	
	Association& association;
	
	std::shared_ptr<DataReceiver> dataReceiver;
	std::shared_ptr<DataSender> dataSender;
};

}

#endif
