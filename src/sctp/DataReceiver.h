#ifndef SCTP_DATARECEIVER_H
#define SCTP_DATARECEIVER_H

#include "SequenceNumberWrapper.h"
#include "Chunk.h"

#include <stdint.h>
#include <set>

namespace sctp
{

class DataReceiver
{
public:
	using TsnWrapper = SequenceNumberWrapper<uint32_t>;

	DataReceiver(uint32_t initialTsn);

	bool HandlePayloadChunk(std::shared_ptr<PayloadDataChunk> chunk);
	
	std::shared_ptr<SelectiveAcknowledgementChunk> generateSackChunk();
	
	inline uint32_t GetLocalAdvertisedReceiverWindowCredit() const
	{
		return localAdvertisedReceiverWindowCredit;
	}
private:
	TsnWrapper receivedTsnWrapper;
	
	uint32_t localAdvertisedReceiverWindowCredit = 0xFFFFFFFF;
	uint64_t cumulativeTsn = 0;
	
	bool initialised = false;
	
	std::multiset<uint64_t> receivedTsns;
};


}

#endif