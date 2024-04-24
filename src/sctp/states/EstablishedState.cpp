#include "EstablishedState.h"
#include "sctp/Association.h"
#include <random>

namespace
{

}

using namespace sctp;

EstablishedState::EstablishedState(Association& association) :
	association(association)
{
	
}
template <typename Event>
void EstablishedState::onEnter(const Event& event, const std::pair<uint32_t, uint32_t>& parameters)
{
	Debug("Enter established state\n");
	
	dataReceiver = std::make_shared<DataReceiver>(parameters.first, association);
	dataSender = std::make_shared<DataSender>(association.GetTimeService(), parameters.second, association);
}

template <typename Event>
void EstablishedState::onLeave(const Event& event)
{
	Debug("Leave established state\n");
	
	// Stop timer
	if (sackTimer)
	{
		sackTimer->Cancel();
		sackTimer.reset();
	}
	
	dataReceiver.reset();
}

fsm::Nothing EstablishedState::handle(const ChunkEvent& event)
{
	auto chunk = event.chunk;
	if (chunk->type == Chunk::Type::PDATA)
	{
		//Get chunk of correct type
		auto pdata = std::static_pointer_cast<PayloadDataChunk>(chunk);
		
		bool sendAckImmediatley = dataReceiver->HandlePayloadChunk(pdata);

		//rfc4960#page-78
		//	When the receiver's advertised window is 0, the receiver MUST drop
		//	any new incoming DATA chunk with a TSN larger than the largest TSN
		//	received so far.  If the new incoming DATA chunk holds a TSN value
		//	less than the largest TSN received so far, then the receiver SHOULD
		//	drop the largest TSN held for reordering and accept the new incoming
		//	DATA chunk.  In either case, if such a DATA chunk is dropped, the
		//	receiver MUST immediately send back a SACK with the current receive
		//	window showing only DATA chunks received and accepted so far.  The
		//	dropped DATA chunk(s) MUST NOT be included in the SACK, as they were
		//	not accepted. 
		
		//We need to acknoledfe
		pendingAcknowledge = true;
		
		//If we need to send it now
		if (sendAckImmediatley)
			//Acknoledge now
			pendingAcknowledgeTimeout = 0ms; 
		else 
			//Create timer
			pendingAcknowledgeTimeout = SackTimeout; 
		
	}
	else if(chunk->type == Chunk::Type::SACK)
	{
		dataSender->handleSackChunk(std::static_pointer_cast<SelectiveAcknowledgementChunk>(chunk));
	}
	
	return fsm::Nothing{};
}

fsm::Nothing EstablishedState::handle(const ProcessedEvent& event)
{	
	//If we need to acknowledge
	if (pendingAcknowledge)
	{
		//If we have to do it now
		if (pendingAcknowledgeTimeout == 0ms)
			//Acknoledge now
			Acknowledge();
		//rfc4960#page-78
		//	Specifically, an
		//	acknowledgement SHOULD be generated for at least every second packet
		//	(not every second DATA chunk) received, and SHOULD be generated
		//	within 200 ms of the arrival of any unacknowledged DATA chunk.

		//If there was already a timeout
		else if (sackTimer.get())
			//We should do sack now
			Acknowledge();
		else
			//Schedule timer
			sackTimer = association.GetTimeService().CreateTimer(pendingAcknowledgeTimeout,[this](...){
				//In the future
				Acknowledge();
			});
	}
	
	return fsm::Nothing{};
}

void EstablishedState::Acknowledge()
{
	//New sack message
	auto sack = dataReceiver->generateSackChunk();
	
	//Send it
	association.Enqueue(sack);
	
	//No need to acknoledge
	pendingAcknowledge = false;
	
	//Reset any pending sack timer
	if (sackTimer)
	{
		//Cancel it
		sackTimer->Cancel();
		//Reset it
		sackTimer = nullptr;
	}
}
