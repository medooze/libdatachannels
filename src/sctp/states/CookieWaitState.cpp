
#include "CookieWaitState.h"
#include "sctp/Association.h"

using namespace sctp;

template<typename Event>
void CookieWaitState::onEnter(const Event& event, InitiationChunk::shared chunk)
{
	initChunk = chunk;
	
	//Reset init retransmissions
	retransmissions = 0;
	
	//Set timer
	timer = association.GetTimeService().CreateTimer(RetransmitTimeout,[=](...){
		association.HandleEvent(TimeoutEvent{});
	});
}

template<typename Event>
void CookieWaitState::onLeave(const Event& event)
{
	// Stop timer
	timer->Cancel();
	timer.reset();
}

fsm::Maybe<fsm::ParameterizedTransitionTo<CookieEchoedState, CookieEchoChunk::shared>> CookieWaitState::handle(const ChunkEvent& event)
{
	auto chunk = event.chunk;
	if (chunk->type == Chunk::Type::INIT_ACK)
	{
		//Get chunk of correct type
		auto initAck = std::static_pointer_cast<InitiationAcknowledgementChunk>(chunk);
		
		//	C) Upon reception of the INIT ACK from "Z", "A" shall stop the T1-
		//	init timer and leave the COOKIE-WAIT state.  "A" shall then send
		//	the State Cookie received in the INIT ACK chunk in a COOKIE ECHO
		//	chunk, start the T1-cookie timer, and enter the COOKIE-ECHOED
		//	state.
		//
		
		//Enqueue new INIT chunk
		auto cookieEcho = std::make_shared<CookieEchoChunk>();
		
		//Copy cookie
		cookieEcho->cookie.SetData(initAck->stateCookie);
		
		///Enquee
		association.Enqueue(std::static_pointer_cast<Chunk>(cookieEcho));
		
		return fsm::ParameterizedTransitionTo<CookieEchoedState, CookieEchoChunk::shared>{cookieEcho};
	}
			
	return fsm::Nothing{};
}


fsm::Maybe<fsm::TransitionTo<ClosedState>> CookieWaitState::handle(const TimeoutEvent& event)
{
	//Retransmit init chunk
	if (retransmissions++<MaxRetransmits)
	{
		//Enquee
		association.Enqueue(initChunk);
		
		//Retry again
		timer->Again(RetransmitTimeout);
	} else {
		//Close
		return fsm::TransitionTo<ClosedState>{};
	}
	
	return fsm::Nothing{};
}