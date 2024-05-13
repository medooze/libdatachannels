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
void EstablishedState::onEnter(const Event& event, const std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>& parameters)
{
	Debug("Enter established state\n");
	
	auto [localInitialTsn, localAdvertisedReceiverWindowCredit, remoteInitialTsn, remoteAdveritsedReceiverWindowCredit] = parameters;
	
	dataReceiver = std::make_shared<DataReceiver>(association.GetTimeService(), association, remoteInitialTsn, localAdvertisedReceiverWindowCredit, association);
	dataSender = std::make_shared<DataSender>(association.GetTimeService(), association, localInitialTsn, remoteAdveritsedReceiverWindowCredit);
	
	association.NotifyEstablished();
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
		dataSender->HandleSackChunk(std::static_pointer_cast<SelectiveAcknowledgementChunk>(chunk));
	}
	
	return fsm::Nothing{};
}

fsm::Nothing EstablishedState::handle(const PacketProcessedEvent& event)
{
	dataReceiver->HanldePacketProcessed();
	
	return fsm::Nothing{};
}

fsm::Nothing EstablishedState::handle(const SendEvent& event)
{
	bool sent = dataSender->Send(event.streamId, event.payload);
	
	if (event.callback)
		(*event.callback)(sent ? EventResult::Success : EventResult::Failed);
	
	return fsm::Nothing{};
}