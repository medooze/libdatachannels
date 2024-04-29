#ifndef SCTP_PAYLOAD_H
#define SCTP_PAYLOAD_H

namespace sctp
{

enum PayloadType
{
	DCEP 		  = 50,
	WebRTCString	  = 51,
	WebRTCBinary	  = 53,
	WebRTCStringEmpty = 56,
	WebRTCBinaryEmpty = 57,
};

struct Payload
{
	PayloadType type;
	Buffer data;
};

}

#endif