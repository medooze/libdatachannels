#ifndef EVENTS_H
#define EVENTS_H

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

};

#endif
