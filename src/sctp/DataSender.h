#ifndef SCTP_DATASENDER_H
#define SCTP_DATASENDER_H

#include "SequenceNumberWrapper.h"
#include "Chunk.h"
#include "Datachannels.h"
#include "Message.h"
#include "Transmitter.h"

#include <stdint.h>
#include <set>
#include <stdlib.h> 

namespace sctp
{
	
class RtoCalculator
{
public:
	static constexpr float InitialAlpha = 1.0f / 8;
	static constexpr float InitialBeta = 1.0f / 4;
	static constexpr uint32_t MaxRto = 60 * 1000;
	static constexpr uint32_t MinRto = 1 * 1000;
	static constexpr uint32_t InitialRto = 1000;
	
	void SetRtt(uint32_t rtt)
	{
		if (srtt == 0)
		{
			srtt = rtt;
			rttvar = rtt / 2;
		}
		else
		{
			rttvar = (1 - beta) * rttvar + beta * abs(int(srtt) - int(rtt));
			srtt = (1 - alpha) * srtt + alpha * rtt;
		}
		
		rto = srtt + 4 * rttvar;
	}
	
	uint32_t GetRto() const
	{
		return rto;
	}
	
	void Backoff()
	{
		rto *= 2;
		rto = std::max(rto, MaxRto);
	}
	
	void Reset()
	{
		if (srtt == 0)
		{
			rto = InitialRto;
		}
		else
		{
			rto = srtt + 4 * rttvar;
		}
	}

private:
	
	uint32_t srtt = 0;
	uint32_t rttvar = 0;
	uint32_t rto = InitialRto;
	
	float alpha = InitialAlpha;
	float beta = InitialBeta;
};
	
class DataSender : public std::enable_shared_from_this<DataSender>
{
public:
	using TsnWrapper = SequenceNumberWrapper<uint32_t>;
	
	static constexpr uint32_t PayloadPoolSize = 1000;
	
	DataSender(datachannels::TimeService& timeService, Transmitter& transmitter, uint32_t localInitialTsn, uint32_t remoteAdvertisedReceiverWindowCredit);
	~DataSender();
	
	bool Send(uint16_t streamId, std::shared_ptr<datachannels::Message> data);
	
	void HandleSackChunk(std::shared_ptr<SelectiveAcknowledgementChunk> chunk);
	
	inline void SetRtt(uint32_t rtt);
	
private:
	void checkRetransmission();
	
	void startRtxTimer(bool restart = false);

	datachannels::TimeService& timeService;
	Transmitter &transmitter;
	uint32_t cumulativeTsnAckPoint = 0;
	
	uint32_t remoteAdveritsedReceiverWindowCredit = 0;
	
	std::set<uint64_t> unackedTsns;
	std::map<uint64_t, std::shared_ptr<PayloadDataChunk>> payloadDataChunks;
	
	std::unordered_map<uint16_t, uint16_t> streamSequenceNumbers;
	
	datachannels::Timer::shared rtxTimer;
	
	TsnWrapper tsnWrapper;
	
	RtoCalculator rtoCalculator;
};
}



#endif
