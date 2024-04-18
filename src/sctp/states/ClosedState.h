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
	
	fsm::Maybe<fsm::TransitionTo<EstablishedState>> handle(const ChunkEvent& event);
	fsm::Maybe<fsm::ParameterizedTransitionTo<CookieWaitState, InitiationChunk::shared>> handle(const AssociateEvent& event);
	
private:
	Association& association;
};

}

#endif
