
#include "CookieEchoedState.h"
#include "sctp/Association.h"

using namespace sctp;


void CookieEchoedState::onEnter(const ChunkEvent& event, CookieEchoChunk::shared chunk)
{
	echoChunk = chunk;
	
	//Reset  retransmissions
	retransmissions = 0;
	
	//Set timer
	timer = association.GetTimeService().CreateTimer(retransmitTimeout,[=](...){
		association.HandleEvent(TimeoutEvent{});
	});
}

void CookieEchoedState::onLeave(const ChunkEvent& event)
{
	// Stop timer
	timer->Cancel();
	timer.reset();
}

fsm::Maybe<fsm::TransitionTo<EstablishedState>> CookieEchoedState::handle(const ChunkEvent& event)
{
	auto chunk = event.chunk;
	if (chunk->type == Chunk::Type::COOKIE_ACK)
	{
		// Stop timer
		timer->Cancel();
		
		return fsm::TransitionTo<EstablishedState>{};
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