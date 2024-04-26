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
	
	dataReceiver = std::make_shared<DataReceiver>(association.GetTimeService(), association, parameters.first, association);
	dataSender = std::make_shared<DataSender>(association.GetTimeService(), association, parameters.second);
}

template <typename Event>
void EstablishedState::onLeave(const Event& event)
{
	Debug("Leave established state\n");
	
	dataSender.reset();
	dataReceiver.reset();
}

fsm::Nothing EstablishedState::handle(const ChunkEvent& event)
{
	auto chunk = event.chunk;
	if (chunk->type == Chunk::Type::PDATA)
	{
		dataReceiver->HandlePayloadChunk(std::static_pointer_cast<PayloadDataChunk>(chunk));		
	}
	else if(chunk->type == Chunk::Type::SACK)
	{
		dataSender->handleSackChunk(std::static_pointer_cast<SelectiveAcknowledgementChunk>(chunk));
	}
	
	return fsm::Nothing{};
}

fsm::Nothing EstablishedState::handle(const PacketProcessedEvent& event)
{
	dataReceiver->HanldePacketProcessed();
	
	return fsm::Nothing{};
}