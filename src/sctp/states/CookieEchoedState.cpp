
#include "CookieEchoedState.h"
#include "sctp/Association.h"

using namespace sctp;

template<typename Event>
void CookieEchoedState::onEnter(const Event& event, CookieEchoChunk::shared chunk)
{
	Debug("Enter cookie echoed state\n");
	
	echoChunk = chunk;
	
	//Reset  retransmissions
	retransmissions = 0;
	
	//Set timer
	timer = association.GetTimeService().CreateTimer(retransmitTimeout,[=](...){
		association.HandleEvent(TimeoutEvent{});
	});
}

template<typename Event>
void CookieEchoedState::onLeave(const Event& event)
{
	Debug("Leave cookie echoed state\n");
	
	// Stop timer
	timer->Cancel();
	timer.reset();
}

fsm::Maybe<fsm::ParameterizedTransitionTo<EstablishedState, std::pair<uint32_t, uint32_t>>> CookieEchoedState::handle(const ChunkEvent& event)
{
	auto chunk = event.chunk;
	if (chunk->type == Chunk::Type::COOKIE_ACK)
	{
		// Stop timer
		timer->Cancel();
		
		// @todo Proper initial tsn
		return fsm::ParameterizedTransitionTo<EstablishedState, std::pair<uint32_t, uint32_t>>{{0, 0}};
	}

	return fsm::Nothing{};
}


fsm::Maybe<fsm::TransitionTo<ClosedState>> CookieEchoedState::handle(const TimeoutEvent& event)
{
	//Retransmit  chunk
	if (retransmissions++ <MaxRetransmits)
	{
		//Enquee
		association.Enqueue(echoChunk);
		//Retry again
		timer->Again(retransmitTimeout);
	} else {
		//Close
		return fsm::TransitionTo<ClosedState>{};
	}
	
	return fsm::Nothing{};
}