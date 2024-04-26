
#include "DataReceiver.h"
#include "Transmitter.h"

namespace sctp
{

DataReceiver::DataReceiver(datachannels::TimeService& timeService, Transmitter& transmitter, uint32_t remoteInitialTsn, 
				uint32_t localAdvertisedReceiverWindowCredit, Listener& listener) :
	timeService(timeService),
	transmitter(transmitter),
	cumulativeTsn(remoteInitialTsn),
	localAdvertisedReceiverWindowCredit(localAdvertisedReceiverWindowCredit),
	listener(listener)
{
}

DataReceiver::~DataReceiver()
{
	// Stop timer
	if (sackTimer)
	{
		sackTimer->Cancel();
		sackTimer.reset();
	}
}

void DataReceiver::HandlePayloadChunk(std::shared_ptr<PayloadDataChunk> chunk)
{
	//Get tsn
	auto tsn = receivedTsnWrapper.Wrap(chunk->transmissionSequenceNumber);
	
	//Storea tsn, if the container has elements with equivalent key, inserts at the upper bound of that range 
	auto it = receivedTsns.insert(tsn);
	
	//	When a packet arrives with duplicate DATA chunk(s) and with no new
	//	DATA chunk(s), the endpoint MUST immediately send a SACK with no
	//	delay.  If a packet arrives with duplicate DATA chunk(s) bundled with
	//	new DATA chunks, the endpoint MAY immediately send a SACK.
	bool duplicated = it != receivedTsns.begin() && *(--it)!=tsn;
	if (!duplicated)
	{
		payloadDataChunks[tsn] = chunk;
	}
	
	//rfc4960#page-89
	//	Upon the reception of a new DATA chunk, an endpoint shall examine the
	//	continuity of the TSNs received.  If the endpoint detects a gap in
	//	the received DATA chunk sequence, it SHOULD send a SACK with Gap Ack
	//	Blocks immediately.  The data receiver continues sending a SACK after
	//	receipt of each SCTP packet that doesn't fill the gap.
	bool hasGaps = false;
	
	//Iterate the received transmission numbers
	uint64_t prev = cumulativeTsn;
	for (auto curr : receivedTsns)
	{
		//Check if not first or if ther is a jump bigger than 1 seq num
		if (curr>prev+1)
		{
			//It has a gap at least
			hasGaps = true;
			break;
		}
		//Next
		prev = curr;
	}
	
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
	if (!initialised || duplicated || hasGaps)
		//Acknoledge now
		pendingAcknowledgeTimeout = 0ms; 
	else 
		//Create timer
		pendingAcknowledgeTimeout = SackTimeout; 
	
	
	initialised = true;
}

void DataReceiver::HanldePacketProcessed()
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
			sackTimer = timeService.CreateTimer(pendingAcknowledgeTimeout,[this](...){
				//In the future
				Acknowledge();
			});
	}	
}

std::shared_ptr<SelectiveAcknowledgementChunk> DataReceiver::generateSackChunk()
{
	//New sack message
	auto sack = std::make_shared<SelectiveAcknowledgementChunk>();
	
	//rfc4960#page-34
	//	By definition, the value of the Cumulative TSN Ack parameter is the
	//	last TSN received before a break in the sequence of received TSNs
	//	occurs; the next TSN value following this one has not yet been
	//	received at the endpoint sending the SACK.  This parameter therefore
	//	acknowledges receipt of all TSNs less than or equal to its value.

	//rfc4960#page-34
	//	The SACK also contains zero or more Gap Ack Blocks.  Each Gap Ack
	//	Block acknowledges a subsequence of TSNs received following a break
	//	in the sequence of received TSNs.  By definition, all TSNs
	//	acknowledged by Gap Ack Blocks are greater than the value of the
	//	Cumulative TSN Ack.
	
	//rfc4960#page-78
	//	Any received DATA chunks with TSN
	//	greater than the value in the Cumulative TSN Ack field are reported
	//	in the Gap Ack Block fields.  The SCTP endpoint MUST report as many
	//	Gap Ack Blocks as can fit in a single SACK chunk limited by the
	//	current path MTU.
	
	//Iterate the received transmission numbers
	uint64_t prev = initialised;
	std::optional<uint64_t> gap;
	for (auto it = receivedTsns.begin();it!=receivedTsns.end();)
	{
		//Get current
		uint64_t current = *it;
		//If we have a continous sequence number or it is the first one
		if (current==cumulativeTsn+1)
		{
			//Update last received tsn
			cumulativeTsn = current;
		}
		//Check if is duplicated
		else if (prev==current)
		{
			//It is duplicated
			sack->duplicateTuplicateTrasnmissionSequenceNumbers.push_back(current);
		}
		//If it is a gap
		else if (current>prev+1)
		{
			//If we had a gap start
			if (gap)
				//Add block ending at previous one
				sack->gapAckBlocks.push_back({
					static_cast<uint16_t>(*gap-cumulativeTsn),
					static_cast<uint16_t>(prev-cumulativeTsn)
				});
			//Start new gap
			gap = current;
		}
		
		//Move next
		prev = current;
		
		//Remove everything up to the cumulative sequence number
		if (cumulativeTsn==current)
		{
			//Erase and move
			it = receivedTsns.erase(it);
			
			// Notify data received
			auto data = payloadDataChunks[current];
			
			auto payload = std::make_unique<Payload>();
			payload->streamId = data->streamIdentifier;
			payload->data = std::move(data->userData);
			payload->type = PayloadType(data->payloadProtocolIdentifier);
			
			listener.OnDataReceived(std::move(payload));
			
			payloadDataChunks.erase(current);
		}
		else
			//Next
			++it;
	}
	
	//If we had a gap start
	if (gap)
		//Add block ending at last one
		sack->gapAckBlocks.push_back({
			static_cast<uint16_t>(*gap-cumulativeTsn),
			static_cast<uint16_t>(prev-cumulativeTsn)
		});
		
	//Set last consecutive recevied number
	sack->cumulativeTrasnmissionSequenceNumberAck = receivedTsnWrapper.UnWrap(cumulativeTsn);
	
	//Set window
	sack->adveritsedReceiverWindowCredit = localAdvertisedReceiverWindowCredit;
	
	return sack;
}

void DataReceiver::Acknowledge()
{
	//New sack message
	auto sack = generateSackChunk();
	
	//Send it
	transmitter.Enqueue(sack);
	
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


}