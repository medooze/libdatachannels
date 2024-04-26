#ifndef CLOSEDSTATE_H
#define CLOSEDSTATE_H

#include "Events.h"
#include "Fsm.h"
#include "sctp/Chunk.h"


namespace sctp
{

class Association;
class EstablishedState;
class CookieWaitState;

class ClosedState : public fsm::ByDefault<fsm::Nothing>
{
public:
	using ByDefault::handle;
	
	ClosedState(Association& association);
	
	template<typename Event> void onEnter(const Event& event);
	template<typename Event> void onLeave(const Event& event);
	
	fsm::Maybe<fsm::ParameterizedTransitionTo<EstablishedState, std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>>> handle(const ChunkEvent& event);
	fsm::Maybe<fsm::ParameterizedTransitionTo<CookieWaitState, InitiationChunk::shared>> handle(const AssociateEvent& event);
	
private:
	Association& association;
	
	uint32_t localInitialTsn = 0;
	uint32_t remoteInitialTsn = 0;
	uint32_t localAdvertisedReceiverWindowCredit = 0xFFFFFFFF;
	uint32_t remoteAdvertisedReceiverWindowCredit = 0;
};

}

#endif
