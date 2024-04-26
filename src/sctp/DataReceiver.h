#ifndef SCTP_DATARECEIVER_H
#define SCTP_DATARECEIVER_H

#include "SequenceNumberWrapper.h"
#include "Chunk.h"
#include "Payload.h"
#include "Datachannels.h"

#include <stdint.h>
#include <set>

using namespace std::literals::chrono_literals;

namespace sctp
{

class Transmitter;

class DataReceiver
{
public:
	using TsnWrapper = SequenceNumberWrapper<uint32_t>;
	static constexpr std::chrono::milliseconds SackTimeout = 100ms;
	
	class Listener
	{
	public:
		virtual ~Listener() = default;
		virtual void OnDataReceived(std::unique_ptr<Payload> data) = 0;
	};
	
	DataReceiver(datachannels::TimeService& timeService, Transmitter& transmitter, uint32_t remoteInitialTsn, uint32_t localAdvertisedReceiverWindowCredit, Listener& listener);
	~DataReceiver();

	void HandlePayloadChunk(std::shared_ptr<PayloadDataChunk> chunk);
	void HanldePacketProcessed();
	
private:
	void Acknowledge();
	
	std::shared_ptr<SelectiveAcknowledgementChunk> generateSackChunk();

	datachannels::TimeService& timeService;
	Transmitter &transmitter;
	uint64_t cumulativeTsn = 0;
	uint32_t localAdvertisedReceiverWindowCredit = 0xFFFFFFFF;
	Listener& listener;
	
	TsnWrapper receivedTsnWrapper;
	bool initialised = false;
	
	std::multiset<uint64_t> receivedTsns;
	std::map<uint64_t, std::shared_ptr<PayloadDataChunk>> payloadDataChunks;
	
	bool pendingAcknowledge = false;
	std::chrono::milliseconds pendingAcknowledgeTimeout = 0ms;
	
	datachannels::Timer::shared sackTimer;
};


}

#endif