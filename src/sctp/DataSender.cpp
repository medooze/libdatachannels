#include "DataSender.h"

namespace sctp
{

DataSender::DataSender(datachannels::TimeService& timeService, Transmitter& transmitter, uint32_t localInitialTsn, uint32_t remoteAdvertisedReceiverWindowCredit) :
	timeService(timeService),
	transmitter(transmitter),
	cumulativeTsnAckPoint(localInitialTsn - 1),
	remoteAdveritsedReceiverWindowCredit(remoteAdvertisedReceiverWindowCredit)
{
}

DataSender::~DataSender()
{
	// Stop timer
	if (rtxTimer)
	{
		rtxTimer->Cancel();
		rtxTimer.reset();
	}
}

bool DataSender::Send(uint16_t streamId, std::shared_ptr<sctp::Payload> data)
{
	std::weak_ptr<DataSender> weak = shared_from_this();
	timeService.Async([weak, streamId, data](std::chrono::milliseconds) {
		auto self = weak.lock();
		if (!self) return;
		
		// Give space for new data if pool is full
		if (self->payloadDataChunks.size() > PayloadPoolSize)
		{
			self->payloadDataChunks.erase(self->payloadDataChunks.begin());
		}
		
		auto chunk = std::make_shared<PayloadDataChunk>();
		chunk->transmissionSequenceNumber = self->unackedTsns.empty() ? 
			(self->cumulativeTsnAckPoint + 1) : *(--self->unackedTsns.end()) + 1;
			
		chunk->streamIdentifier = streamId;
		chunk->payloadProtocolIdentifier = uint32_t(data->type);
		chunk->userData = std::move(data->data);
		// We are not supporting fragmentation yet. Set it as a sole complete chunk.
		chunk->beginingFragment = true;
		chunk->endingFragment = true;
		
		if (self->streamSequenceNumbers.find(streamId) == self->streamSequenceNumbers.end())
		{
			self->streamSequenceNumbers[streamId] = 0;
		}
		else
		{
			self->streamSequenceNumbers[streamId] = self->streamSequenceNumbers[streamId] + 1;
		}
		
		chunk->streamSequenceNumber = self->streamSequenceNumbers[streamId];
		
		auto wrapped = self->tsnWrapper.Wrap(chunk->transmissionSequenceNumber);
		self->payloadDataChunks[wrapped] = chunk;
		self->unackedTsns.insert(wrapped);
		
		if (chunk->transmissionSequenceNumber == self->cumulativeTsnAckPoint + 1)
		{
			self->transmitter.Enqueue(chunk);
			Debug("Enqueued payload %d\n", chunk->userData.GetSize());
			
			self->startRtxTimer();
		}
	});
	
	// @todo check if receiver is OK to receive.
	return true;
}

void DataSender::SetRtt(uint32_t rtt)
{
	rtoCalculator.SetRtt(rtt);
}

void DataSender::checkRetransmission()
{
	assert(!unackedTsns.empty());
	
	// @todo Until we implement the proper RTT measurement, don't do backoff
	// rtoCalculator.Backoff();
	
	// @todo We need check whether it can be fit in on packet and whether the peer's rwind indicate no buffer
	std::vector<std::shared_ptr<Chunk>> bundle;
	for (auto tsn : unackedTsns)
	{
		if (payloadDataChunks.find(tsn) != payloadDataChunks.end())
		{
			bundle.push_back(payloadDataChunks[tsn]);
		}
	}
	
	if (!bundle.empty())
	{
		Debug("Retransmittion: %d\n", bundle.size());
		transmitter.Enqueue(bundle);
		
		startRtxTimer();
	}
	
}
	
void DataSender::HandleSackChunk(std::shared_ptr<SelectiveAcknowledgementChunk> chunk)
{
	remoteAdveritsedReceiverWindowCredit = chunk->adveritsedReceiverWindowCredit;
	
	auto wrappedCumulativeTsnAck = tsnWrapper.Wrap(chunk->cumulativeTrasnmissionSequenceNumberAck);
	auto wrappedCumulativeTsnAckPoint = tsnWrapper.Wrap(cumulativeTsnAckPoint);
	
	// If Cumulative TSN Ack is less than the Cumulative TSN Ack Point, then drop the SACK chunk.
	if (wrappedCumulativeTsnAck < wrappedCumulativeTsnAckPoint && 
	    (wrappedCumulativeTsnAck > TsnWrapper::OutOfOrderWindow || wrappedCumulativeTsnAckPoint == 0xFFFFFFFF))
	{
		Debug("Unexpected TSN: %ld cumulativeTsnAckPoint: %ld\n", chunk->cumulativeTrasnmissionSequenceNumberAck, cumulativeTsnAckPoint);
		return;
	}
	
	cumulativeTsnAckPoint = chunk->cumulativeTrasnmissionSequenceNumberAck;
	
	std::optional<uint64_t> earlistTsn = 0;
	if (!unackedTsns.empty())
	{
		earlistTsn = *unackedTsns.begin();
	}
	
	for (auto it = unackedTsns.begin(); it!= unackedTsns.end();)
	{
		if (*it <= wrappedCumulativeTsnAck)
		{
			it = unackedTsns.erase(it);
		}
		else
		{
			break;
		}
	}
	
	bool missing = false;
	uint32_t pos = 0;
	for (auto& gap : chunk->gapAckBlocks)
	{
		auto start = wrappedCumulativeTsnAckPoint + gap.first;
		auto end = wrappedCumulativeTsnAckPoint + gap.second;
		
		while (pos++ < start)
		{
			auto missingTsn = wrappedCumulativeTsnAckPoint + pos;
			
			if (payloadDataChunks.find(missingTsn) != payloadDataChunks.end())
			{
				auto [it, inserted] = unackedTsns.insert(wrappedCumulativeTsnAckPoint + pos);
				missing = inserted;
			}
		}
		
		for (auto i = start; i <= end; i++)
		{
			unackedTsns.erase(i);
		}
		
		pos = end + 1;
	}
	
	if (!chunk->duplicateTuplicateTrasnmissionSequenceNumbers.empty())
	{
		Debug("Receiver got duplicated tsns.\n");
	}
	
	Debug("Unacked tsns: %zu, cumulativeTsnAckPoint: %u \n", unackedTsns.size(), cumulativeTsnAckPoint);
	
	if (missing)
	{
		// Whenever a SACK chunk is received missing a TSN that was previously 
		// acknowledged via a Gap Ack Block, start the T3-rtx for the destination 
		// address to which the DATA chunk was originally transmitted if it is not 
		// already running
		startRtxTimer();
	}
	else if (earlistTsn && !unackedTsns.empty() && *earlistTsn != *unackedTsns.begin())
	{
		// Whenever a SACK chunk is received that acknowledges the DATA chunk with 
		// the earliest outstanding TSN for that address, restart the T3-rtx timer 
		// for that address with its current RTO (if there is still outstanding data
		// on that address)
		startRtxTimer(true);
	}
	else if (unackedTsns.empty())
	{
		if (rtxTimer) rtxTimer->Cancel();
	}
}


void DataSender::startRtxTimer(bool restart)
{
	std::chrono::milliseconds retransimissionTimeOut{rtoCalculator.GetRto()};
	Debug("Start rtx timer: %d\n", retransimissionTimeOut.count());
	
	if (!rtxTimer)
	{
		rtxTimer = timeService.CreateTimer(retransimissionTimeOut,[this](...){
				checkRetransmission();
			});
	}
	else if (restart || !rtxTimer->IsScheduled())
	{
		rtxTimer->Again(retransimissionTimeOut);
	}
}

}