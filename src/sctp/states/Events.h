#ifndef EVENTS_H
#define EVENTS_H

#include "sctp/Payload.h"

#include <memory>
#include <functional>

namespace sctp
{

struct ChunkEvent
{
	Chunk::shared chunk;
};

struct PacketProcessedEvent
{	
};

struct AssociateEvent
{
};

struct ShutdownEvent
{	
};

struct AbortEvent
{	
};

struct TimeoutEvent 
{
};

struct SendEvent
{
	enum class ProcessResult
	{
		Unprocessed,
		Success,
		Failed	
	};
	
	std::shared_ptr<sctp::Payload> payload;
	std::function<void(ProcessResult)> callback;	
};

};
#endif
