#ifndef DATACHANNELS_MESSAGE_H
#define DATACHANNELS_MESSAGE_H

#include <cstdint>
#include <cstring>
#include <stddef.h>
#include <memory>
#include <stdlib.h>
#include <vector>

namespace datachannels {

enum MessageType
{
	DCEP 		  = 50,
	WebRTCString	  = 51,
	WebRTCBinary	  = 53,
	WebRTCStringEmpty = 56,
	WebRTCBinaryEmpty = 57,
};

struct Message
{
	MessageType type;
	std::vector<uint8_t> data;
};

class MessageListener
{
public:
	virtual ~MessageListener() = default;
	virtual void OnMessage(const std::shared_ptr<Message>& message) = 0;
};

class MessageProducer
{
public:
	virtual ~MessageProducer() = default;
	virtual void AddMessageListener(const std::shared_ptr<MessageListener>& listener) = 0;
	virtual void RemoveMessageListener(const std::shared_ptr<MessageListener>& listener) = 0;
};

}


#endif