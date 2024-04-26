#ifndef SCTP_PAYLOAD_H
#define SCTP_PAYLOAD_H

namespace sctp
{

enum PayloadType
{
	DECP 		  = 50,
	WebRTCString	  = 51,
	WebRTCBinary	  = 53,
	WebRTCStringEmpty = 56,
	WebRTCBinaryEmpty = 57,
};

struct Payload
{
	uint16_t streamId;
	PayloadType type;
	Buffer data;
};

}

#endif