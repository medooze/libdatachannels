#ifndef COOKIEWAITSTATE_H
#define COOKIEWAITSTATE_H

#include "Events.h"
#include "Fsm.h"
#include "sctp/Chunk.h"
#include "Datachannels.h"

#include <chrono>

using namespace std::chrono_literals;

namespace sctp
{

class Association;
class CookieEchoedState;
class ClosedState;

class CookieWaitState : public fsm::ByDefault<fsm::Nothing>
{
public:
	using ByDefault::handle;

	static constexpr const size_t MaxRetransmits = 10;
	static constexpr const std::chrono::milliseconds RetransmitTimeout = 100ms;
	
	CookieWaitState(Association& association) : association(association) {};
	
	template<typename Event> void onEnter(const Event& event, InitiationChunk::shared chunk);
	template<typename Event> void onLeave(const Event& event);
	
	fsm::Maybe<fsm::ParameterizedTransitionTo<CookieEchoedState, CookieEchoChunk::shared>> handle(const ChunkEvent& event);
		
	fsm::Maybe<fsm::TransitionTo<ClosedState>> handle(const TimeoutEvent& event);
	
private:
	Association& association;
	
	InitiationChunk::shared initChunk;
	
	uint32_t retransmissions = 0;
	datachannels::Timer::shared timer;
};

}

#endif
