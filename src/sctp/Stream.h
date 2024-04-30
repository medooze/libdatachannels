#ifndef SCTP_STREAM_H
#define SCTP_STREAM_H

#include "Datachannels.h"

#include <list>
#include <memory>

#include "Buffer.h"
#include "Payload.h"
#include <memory>


namespace sctp
{

class Association;
	
class Stream
{
public:
	using shared = std::shared_ptr<Stream>;
public:
	class Listener
	{
	public:
		virtual ~Listener() = default;
		virtual void OnPayload(std::unique_ptr<sctp::Payload> payload) = 0;
	};

	Stream(Association &association, uint16_t id);
	virtual ~Stream();
	
	bool Recv(std::unique_ptr<sctp::Payload> payload);
	bool Send(std::unique_ptr<sctp::Payload> payload);
	
	uint16_t GetId() const { return id; }
	
	void SetListener(Listener* listener);
	
	inline Association& getAssociation()
	{
		return association;
	}
	
private:
	uint16_t id;
	Association &association;
	
	Listener* listener = nullptr;
};

}; // namespace
#endif /* SCTP_STREAM_H */

