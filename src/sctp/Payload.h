#ifndef SCTP_PAYLOAD_H
#define SCTP_PAYLOAD_H

enum PayloadType
{
	DECP = 50,
};

struct Payload
{
	uint16_t streamId;
	PayloadType type;
	Buffer data;
};

#endif