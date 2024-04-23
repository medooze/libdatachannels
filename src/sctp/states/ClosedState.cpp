#include "ClosedState.h"
#include "sctp/Association.h"
#include <random>

namespace
{
//Random stuff
std::random_device rd;
std::mt19937 gen{rd()};
std::uniform_int_distribution<unsigned long> dis{1, 4294967295};
}

using namespace sctp;

ClosedState::ClosedState(Association& association) :
	association(association)
{
	
}

template<typename Event>
void ClosedState::onEnter(const Event& event)
{
	Debug("Enter closed state\n");
	association.SetLocalVerificationTag(0);
}

template<typename Event>
void ClosedState::onLeave(const Event& event)
{
	Debug("Leave closed state\n");
}

fsm::Maybe<fsm::ParameterizedTransitionTo<EstablishedState, std::pair<uint32_t, uint32_t>>> ClosedState::handle(const ChunkEvent& event)
{
	auto chunk = event.chunk;
	if (chunk->type == Chunk::Type::INIT)
	{
		//Get chunk of correct type
		auto init = std::static_pointer_cast<InitiationChunk>(chunk);
		initialTsn = init->initialTransmissionSequenceNumber;
		
		//rfc4960#page-55
		//	"Z" shall respond immediately with an INIT ACK chunk.  The
		//	destination IP address of the INIT ACK MUST be set to the source
		//	IP address of the INIT to which this INIT ACK is responding.  In
		//	the response, besides filling in other parameters, "Z" must set
		//	the Verification Tag field to Tag_A, and also provide its own
		//	Verification Tag (Tag_Z) in the Initiate Tag field.
		//
		
		association.SetRemoteVerificationTag(init->initiateTag);
		
		//Create new verification tag
		auto localVerificationTag = dis(gen);
		
		association.SetLocalVerificationTag(localVerificationTag);

		//Enqueue new INIT chunk
		auto initAck = std::make_shared<InitiationAcknowledgementChunk>();

		//Set params
		initAck->initiateTag			= localVerificationTag;
		initAck->advertisedReceiverWindowCredit	= association.GetLocalAdvertisedReceiverWindowCredit();
		initAck->numberOfOutboundStreams	= 0xFFFF;
		initAck->numberOfInboundStreams		= 0xFFFF;
		initAck->initialTransmissionSequenceNumber = 0;

		// draft-ietf-rtcweb-data-channel-13
		//	The INIT and INIT-ACK chunk MUST NOT contain any IPv4 Address or
		//	IPv6 Address parameters.  The INIT chunk MUST NOT contain the
		//	Supported Address Types parameter.

		// draft-ietf-rtcweb-data-channel-13#page-7
		//	The dynamic address reconfiguration extension defined in [RFC5061]
		//	MUST be used to signal the support of the stream reset extension
		//	defined in [RFC6525].  Other features of [RFC5061] are OPTIONAL.
		initAck->supportedExtensions.push_back(Chunk::Type::RE_CONFIG);
		
		//rfc4960#page-55
		//	Moreover, "Z" MUST generate and send along with the INIT ACK a
		//	State Cookie.  See Section 5.1.3 for State Cookie generation.
		
		// AAA is ensured by the DTLS & ICE layer, so createing a complex cookie is unnecessary IMHO
		initAck->stateCookie.SetData((uint8_t*)"dtls",strlen("dtls"));

		//Send back unkown parameters
		for (const auto& unknown : init->unknownParameters)
			//Copy as unrecognized
			initAck->unrecognizedParameters.push_back(unknown.second.Clone());
		
		///Enquee
		association.Enqueue(std::static_pointer_cast<Chunk>(initAck));
	}
	else if (chunk->type == Chunk::Type::COOKIE_ECHO)
	{
		//rfc4960#page-55
		//	D) Upon reception of the COOKIE ECHO chunk, endpoint "Z" will reply
		//	with a COOKIE ACK chunk after building a TCB and moving to the
		//	ESTABLISHED state.  A COOKIE ACK chunk may be bundled with any
		//	pending DATA chunks (and/or SACK chunks), but the COOKIE ACK chunk
		//	MUST be the first chunk in the packet.
		
		//Enqueue new INIT chunk
		auto cookieAck = std::make_shared<CookieAckChunk>();
		
		//We don't check the cookie for seame reasons as we don't create one
		
		///Enquee
		association.Enqueue(std::static_pointer_cast<Chunk>(cookieAck));
		
		return fsm::ParameterizedTransitionTo<EstablishedState, std::pair<uint32_t, uint32_t>>{{initialTsn, 0}};
	}
	
	return fsm::Nothing{};
}

fsm::Maybe<fsm::ParameterizedTransitionTo<CookieWaitState, InitiationChunk::shared>> ClosedState::handle(const AssociateEvent& event)
{
	//	"A" first sends an INIT chunk to "Z".  In the INIT, "A" must
	//	provide its Verification Tag (Tag_A) in the Initiate Tag field.
	//	Tag_A SHOULD be a random number in the range of 1 to 4294967295
	//	(see Section 5.3.1 for Tag value selection).  After sending the
	//	INIT, "A" starts the T1-init timer and enters the COOKIE-WAIT
	//	state.
	
	//Create new verification tag
	auto localVerificationTag = dis(gen);
	association.SetLocalVerificationTag(localVerificationTag);
	
	//Enqueue new INIT chunk
	auto init = std::make_shared<InitiationChunk>();
	
	//Set params
	init->initiateTag			= localVerificationTag;
	init->advertisedReceiverWindowCredit	= 0;
	init->numberOfOutboundStreams		= 0xFFFF;
	init->numberOfInboundStreams		= 0xFFFF;
	init->initialTransmissionSequenceNumber = 0;
	
	// draft-ietf-rtcweb-data-channel-13
	//	The INIT and INIT-ACK chunk MUST NOT contain any IPv4 Address or
	//	IPv6 Address parameters.  The INIT chunk MUST NOT contain the
 	//	Supported Address Types parameter.
	
	// draft-ietf-rtcweb-data-channel-13#page-7
	//	The dynamic address reconfiguration extension defined in [RFC5061]
	//	MUST be used to signal the support of the stream reset extension
	//	defined in [RFC6525].  Other features of [RFC5061] are OPTIONAL.
	init->supportedExtensions.push_back(Chunk::Type::RE_CONFIG);
				
	association.Enqueue(std::static_pointer_cast<Chunk>(init));
	
	return fsm::ParameterizedTransitionTo<CookieWaitState, InitiationChunk::shared>{init};
}
