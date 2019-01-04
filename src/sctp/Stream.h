#ifndef SCTP_STREAM_H
#define SCTP_STREAM_H

#include "datachannels.h"

#include <list>
#include <memory>

#include "Buffer.h"


namespace sctp
{

class Association;
	
class Stream
{
public:
	using shared = std::shared_ptr<Stream>;
public:
	Stream(Association &association, uint16_t id);
	virtual ~Stream();
	
	bool Send(const uint8_t ppid, const uint8_t* buffer, const size_t size);
	
	uint16_t GetId() const { return id; }
private:
	uint16_t id;
	Association &association;
	std::list<std::pair<uint8_t,Buffer>> outgoingMessages;
	Buffer incomingMessage;
};

}; // namespace
#endif /* SCTP_STREAM_H */

