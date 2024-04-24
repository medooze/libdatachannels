#ifndef SCTP_DATARECEIVER_H
#define SCTP_DATARECEIVER_H

#include "SequenceNumberWrapper.h"
#include "Chunk.h"
#include "Payload.h"

#include <stdint.h>
#include <set>

namespace sctp
{

class DataReceiver
{
public:
	using TsnWrapper = SequenceNumberWrapper<uint32_t>;
	
	class Listener
	{
	public:
		virtual ~Listener() = default;
		virtual void OnDataReceived(std::unique_ptr<Payload> data) = 0;
	};
	
	DataReceiver(uint32_t initialTsn, Listener& listener);

	bool HandlePayloadChunk(std::shared_ptr<PayloadDataChunk> chunk);
	
	inline uint32_t GetLocalAdvertisedReceiverWindowCredit() const
	{
		return localAdvertisedReceiverWindowCredit;
	}
	
	std::shared_ptr<SelectiveAcknowledgementChunk> generateSackChunk();
private:

	Listener& listener;
	
	TsnWrapper receivedTsnWrapper;
	
	uint32_t localAdvertisedReceiverWindowCredit = 0xFFFFFFFF;
	uint64_t cumulativeTsn = 0;
	
	bool initialised = false;
	
	std::multiset<uint64_t> receivedTsns;
	std::map<uint64_t, std::shared_ptr<PayloadDataChunk>> payloadDataChunks;
};


}

#endif