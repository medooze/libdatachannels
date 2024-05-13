#ifndef SCTP_STREAM_H
#define SCTP_STREAM_H

#include "Datachannels.h"

#include <list>
#include <memory>

#include "Buffer.h"
#include "Message.h"
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
		virtual void OnPayload(std::shared_ptr<datachannels::Message> payload) = 0;
	};

	Stream(Association &association, uint16_t id);
	virtual ~Stream();
	
	bool Recv(std::shared_ptr<datachannels::Message> payload);
	bool Send(std::shared_ptr<datachannels::Message> payload);
	
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

