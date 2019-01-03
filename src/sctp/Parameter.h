#ifndef SCTP_CHUNKPARAMETER_H_
#define SCTP_CHUNKPARAMETER_H_
#include <stdint>
#include <string>

namespace sctp
{
namespace Parameter
{
	enum Type
	{
		IPv4Address				: 5,
		IPv6Address				: 6,
		StateCookie				: 7,
		UnrecognizedParameter			: 8,
		CookiePreservative			: 9,
		ReservedforECNCapable:			: 32768,
		HostNameAddress				: 11,
		SupportedAddressTypes			: 12
		OutgoingSSNResetRequestParameter	: 13,
		IncomingSSNResetRequestParameter	: 14,
		SSNTSNResetRequestParameter		: 15,
		ReCconfigurationResponseParameter	: 16,
		AddOutgoingStreamsRequestParameter	: 17,
		AddIncomingStreamsRequestParameter	: 18,
		SupportedExtensions			: 0x8008, //rfc5061
	};

	namespace IPv4Address
	{
		size_t GetSize()	{ return 12;	}
		
	};
};

}; // namespace sctp

#endif

/*
//Depending on the parameter type
switch(paramType)
{
	case Parameter::IPv4Address				:
	case Parameter::IPv6Address				:
	case Parameter::StateCookie				:
	case Parameter::UnrecognizedParameter			:
	case Parameter::CookiePreservative			:
	case Parameter::ReservedforECNCapable:			:
	case Parameter::HostNameAddress				:
	case Parameter::OutgoingSSNResetRequestParameter	:
	case Parameter::IncomingSSNResetRequestParameter	:
	case Parameter::SSNTSNResetRequestParameter		:
	case Parameter::ReCconfigurationResponseParameter	:
	case Parameter::AddOutgoingStreamsRequestParameter	:
	case Parameter::AddIncomingStreamsRequestParameter	:
	case Parameter::SupportedExtensions			:
}
*/
