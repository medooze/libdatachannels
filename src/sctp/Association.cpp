#include "sctp/Association.h"
#include "sctp/Chunk.h"

#include <chrono>
#include <random>
#include <crc32c/crc32c.h>
#include <condition_variable>

namespace sctp
{
	
//Random stuff
std::random_device rd;
std::mt19937 gen{rd()};
std::uniform_int_distribution<unsigned long> dis{1, 4294967295};

Association::Association(datachannels::TimeService& timeService, datachannels::OnTransmissionPendingListener &listener) :
	timeService(timeService),
	dataPendingListener(listener),
	closedState(*this),
	cookieWaitState(*this),
	cookieEchoedState(*this),
	establishedState(*this),
	fsm(closedState, cookieWaitState, cookieEchoedState, establishedState)
{
}

Association::~Association()
{
	fsm.handle(ShutdownEvent{});
	
	// Not clean shutdown. Clean shutdown should be through Shutdown() call and wait for state change
}

bool Association::Associate()
{
	fsm.handle(AssociateEvent{});
	
	return true;
}

bool Association::Shutdown()
{
	fsm.handle(ShutdownEvent{});
	
	return true;
}

bool Association::Abort()
{
	fsm.handle(AbortEvent{});
	
	return true;
}

size_t Association::WritePacket(uint8_t *data, uint32_t size)
{
	//Create reader
	BufferReader reader(data,size);
	
	//TODO: Check crc 
	
	//Parse packet header
	auto header = PacketHeader::Parse(reader);

	//Ensure it was correctly parsed
	if (!header)
	{
		//Error
		Debug("-Association::WritePacket() | header parse failed.");
		return false;
	}

	//Check correct local and remote port
	if (header->sourcePortNumber!=remotePort || header->destinationPortNumber!=localPort || header->verificationTag!=localVerificationTag)
	{
		//Error
		Debug("-Association::WritePacket() | unexpected header. expected: [%d,%d,%x] received: [%d,%d,%x]\n", 
			remotePort, localPort, localVerificationTag,
			header->sourcePortNumber, header->destinationPortNumber, header->verificationTag);
			
		return false;
	}
	
	//Read chunks
	while (reader.GetLeft()>4)
	{
		//Parse chunk
		auto chunk = Chunk::Parse(reader);
		//Check 
		if (!chunk)
		{
			//Error
			fsm.handle(PacketProcessedEvent{});
			return false;
		}
		
		//Process it
		fsm.handle(ChunkEvent{std::nullopt, chunk});
	}
	
	fsm.handle(PacketProcessedEvent{});
	
	//Done
	return true;
}

size_t Association::ReadPacket(uint8_t *data, uint32_t size)
{
	//Check there is pending data
	if (!pendingData)
		//Nothing to do
		return 0;
	
	//Create buffer writter
	BufferWritter writter(data,size);
	
	//Create new packet header
	PacketHeader header(localPort,remotePort,remoteVerificationTag);
	
	//Serialize it
	if (!header.Serialize(writter))
		//Error
		return 0;

	size_t num = 0;
	
	//Fill chunks from control queue first
	for (auto it=queue.begin();it!=queue.end();)
	{
		//Get chunk
		auto chunk = *it;

		//Ensure we have enought space for chunk
		if (writter.GetLeft()<chunk->GetSize())
			//We cant send more on this packet
			break;
		
		//Check if it must be sent alone
		if (chunk->type==Chunk::Type::INIT || chunk->type==Chunk::Type::INIT_ACK || chunk->type==Chunk::Type::COOKIE_ECHO)
		{
			//IF it is not firest
			if (num)
				//Flush all before this
				break;
		}
		
		//Remove from queue and move to next chunk
		it = queue.erase(it);
		
		//Serialize chunk
		chunk->Serialize(writter);
		
		//Check if it must be sent alone
		if (chunk->type==Chunk::Type::INIT || chunk->type==Chunk::Type::INIT_ACK || chunk->type==Chunk::Type::COOKIE_ECHO)
			//Send alone
			break;
	}

	//TODO:Check in which stata data can be sent
		//TODO:Now fill data chunks from streams

	//Get length
	size_t length = writter.GetLength();
	//Calculate crc
	header.checksum  = crc32c::Crc32c(data,length);
	//Go to the begining
	writter.GoTo(0);
	
	//Serialize it now with checksum
	header.Serialize(writter);
	
	//Check if there is more data to send
	if (!queue.size())
		//No
		pendingData = false;
	//Done
	return length;
}

void Association::Enqueue(const Chunk::shared& chunk)
{
	bool wasPending = pendingData;
	//Push back
	queue.push_back(chunk);
	//Reset flag
	pendingData = true;
	//If it is first
	if (!wasPending)
		dataPendingListener.OnTransmissionPending();
}

void Association::Enqueue(const std::vector<Chunk::shared>& chunkBundle)
{
	if (chunkBundle.empty()) return;
	
	bool wasPending = pendingData;
	//Push back
	queue.insert(queue.end(), chunkBundle.begin(), chunkBundle.end());
	//Reset flag
	pendingData = true;
	//If it is first
	if (!wasPending)
		dataPendingListener.OnTransmissionPending();
}

void Association::OnDataReceived(uint16_t streamId, std::unique_ptr<sctp::Payload> data)
{
	if (streams.find(streamId) == streams.end())
	{
		streams[streamId] = std::make_shared<Stream>(*this, streamId);
		// OnStreamCreated
		if (listener)
		{
			listener->OnStreamCreated(streams[streamId]);
		}
	}
	
	streams[streamId]->Recv(std::move(data));
}

bool Association::SendData(uint16_t streamId, std::unique_ptr<sctp::Payload> data)
{
	SendEvent event;
	event.streamId = streamId;
	event.payload = std::move(data);
	
	EventResult result = EventResult::Unprocessed;
	event.callback = [&result](EventResult res){
		result = res;
	};
	
	fsm.handle(event);
	
	return result == EventResult::Success;
}

Stream::shared Association::createStream(uint16_t streamId)
{
	if (streams.find(streamId) == streams.end())
	{
		streams[streamId] = std::make_shared<Stream>(*this, streamId);
		// OnStreamCreated
		if (listener)
		{
			listener->OnStreamCreated(streams[streamId]);
		}
	}
	
	return streams[streamId];
}

}; //namespace sctp
