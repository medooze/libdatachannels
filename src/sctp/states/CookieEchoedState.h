#ifndef COOKIEECHOEDSTATE_H
#define COOKIEECHOEDSTATE_H

#include "Events.h"
#include "Fsm.h"
#include "sctp/Chunk.h"
#include "Datachannels.h"

#include <chrono>

using namespace std::chrono_literals;

namespace sctp
{

class Association;
class EstablishedState;
class ClosedState;

class CookieEchoedState : public fsm::ByDefault<fsm::Nothing>
{
public:
	using ByDefault::handle;

	static constexpr const size_t MaxRetransmits = 10;
	static constexpr const std::chrono::milliseconds retransmitTimeout	= 100ms;
	
	CookieEchoedState(Association& association) : association(association) {};
	
	template<typename Event> void onEnter(const Event& event, CookieEchoChunk::shared chunk);
	template<typename Event> void onLeave(const Event& event);
	
	fsm::Maybe<fsm::TransitionTo<EstablishedState>> handle(const ChunkEvent& event);
	fsm::Maybe<fsm::TransitionTo<ClosedState>> handle(const TimeoutEvent& event);
	
private:
	Association& association;
	
	CookieEchoChunk::shared echoChunk;
	
	uint32_t retransmissions = 0;
	datachannels::Timer::shared timer;
};

}

#endif
