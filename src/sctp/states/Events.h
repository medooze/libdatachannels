#ifndef EVENTS_H
#define EVENTS_H

#include "sctp/Payload.h"

#include <memory>
#include <functional>

namespace sctp
{

enum class EventResult
{
	Unprocessed,
	Success,
	Failed	
};

struct Event
{
	std::optional<std::function<void(EventResult)>> callback;	
};

struct ChunkEvent : public Event
{
	Chunk::shared chunk;
};

struct PacketProcessedEvent : public Event
{
};

struct AssociateEvent : public Event
{
};

struct ShutdownEvent : public Event
{
};

struct AbortEvent : public Event
{	
};

struct TimeoutEvent : public Event
{
};

struct SendEvent : public Event
{
	std::shared_ptr<sctp::Payload> payload;
};

};
#endif
