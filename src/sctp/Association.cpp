#include "sctp/Association.h"
#include "sctp/Chunk.h"

#include <chrono>
#include <random>
#include <crc32c/crc32c.h>

using namespace std::chrono_literals;

namespace sctp
{
	
//Random stuff
std::random_device rd;
std::mt19937 gen{rd()};
std::uniform_int_distribution<unsigned long> dis{1, 4294967295};

#define MaxInitRetransmits 10

Association::Association(datachannels::TimeService& timeService) :
	timeService(timeService)
{
	
}

void Association::SetState(State state)
{
	this->state = state;
}

bool Association::Associate()
{
	//Check state
	if (state!=State::Closed)
		//Error
		return false;
	
	//	"A" first sends an INIT chunk to "Z".  In the INIT, "A" must
	//	provide its Verification Tag (Tag_A) in the Initiate Tag field.
	//	Tag_A SHOULD be a random number in the range of 1 to 4294967295
	//	(see Section 5.3.1 for Tag value selection).  After sending the
	//	INIT, "A" starts the T1-init timer and enters the COOKIE-WAIT
	//	state.
	
	//Create new verification tag
	localVerificationTag = dis(gen);
	
	//Reset init retransmissions
	initRetransmissions = 0;
	
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
		
	//Set timer
	initTimer = timeService.CreateTimer(100ms,[&](...){
		//Retransmit init chunk
		if (initRetransmissions++<MaxInitRetransmits)
		{
			//Enquee
			Enqueue(std::static_pointer_cast<Chunk>(init));
			//Retry again
			initTimer->Again(100ms);
		} else {
			//Close
			SetState(State::Closed);
		}
	});
	
	//Change state
	SetState(State::CookieWait);
	
	//Enquee
	Enqueue(std::static_pointer_cast<Chunk>(init));
	
	//Done
	return true;
}

bool Association::Shutdown()
{
	return true;
}

bool Association::Abort()
{
	return true;
}

size_t Association::WritePacket(uint8_t *data, uint32_t size)
{
	//Create reader
	BufferReader reader(data,size);
	
	//TODO: Check crc 
	
	//Parse packet header
	auto header = PacketHeader::Parse(reader);

	//Ensure it was correctly parsed
	if (!header)
		//Error
		return false;

	//Check correct local and remote port
	if (header->sourcePortNumber!=remotePort || header->destinationPortNumber!=localPort || header->verificationTag==localVerificationTag)
		//Error
		return false;
	
	//Read chunks
	while (reader.GetLeft()>4)
	{
		//Parse chunk
		auto chunk = Chunk::Parse(reader);
		//Check 
		if (!chunk)
			//Skip
			continue;
		//Process it
		Process(chunk);
	}
	
	//Done
	return true;
}

size_t Association::ReadPacket(uint8_t *data, uint32_t size)
{
	//Create buffer writter
	BufferWritter writter(data,size);
	
	//Create new packet header
	PacketHeader header(localPort,remotePort,remoteVerificationTag);
	
	//Serialize it
	if (!header.Serialize(writter))
		//Error
		return 0;

	size_t num = 0;
	
	//Fill chunks from control queue first
	for (auto it=queue.begin();it!=queue.end();)
	{
		//Get chunk
		auto chunk = *it;
		//Check if it must be sent alone
		if (chunk->type==Chunk::Type::INIT || chunk->type==Chunk::Type::INIT_ACK || chunk->type==Chunk::Type::COOKIE_ECHO)
		{
			//Ensure it is the first
			if (!num)
				//Flush all before this
				break;
		}
		//Ensure we have enought space for chunk anc crc
		if (writter.GetLeft()>chunk->GetSize()+4)
			//We cant send more on this packet
			break;
		
		//Remove from queue and move to next chunk
		queue.erase(++it);
		//Serialize chunk
		chunk->Serialize(writter);
		//Add chunk
		num++;

	}

	//TODO:Now fill data chunks from streams

	//Ensure we can add crc
	if (!writter.Assert(4))
		//Error
		return 0;
	
	//Get length
	size_t length = writter.GetLength();
	//Calculate crc
	header.checksum  = crc32c::Extend(0,data,length);
	//Go to the begining
	writter.GoTo(0);
	
	//Serialize it now with checksum
	header.Serialize(writter);
	
	//Check if there is more data to send
	if (!queue.size())
		//No
		pendingData = false;
	//Done
	return length;
}

void Association::Process(const Chunk::shared& chunk)
{
	//Depending onthe state
	switch (state)
	{
		case State::Closed:
		{
			switch(chunk->type)
			{
				case Chunk::Type::INIT:
				{
					//Get chunk of correct type
					auto init = std::static_pointer_cast<InitiationChunk>(chunk);
					
					//rfc4960#page-55
					//	"Z" shall respond immediately with an INIT ACK chunk.  The
					//	destination IP address of the INIT ACK MUST be set to the source
					//	IP address of the INIT to which this INIT ACK is responding.  In
					//	the response, besides filling in other parameters, "Z" must set
					//	the Verification Tag field to Tag_A, and also provide its own
					//	Verification Tag (Tag_Z) in the Initiate Tag field.
					//
					
					//Get remote verification tag
					remoteVerificationTag = init->initiateTag;
						
					//Create new verification tag
					localVerificationTag = dis(gen);

					//Reset init retransmissions
					initRetransmissions = 0;

					//Enqueue new INIT chunk
					auto initAck = std::make_shared<InitiationAcknowledgementChunk>();

					//Set params
					initAck->initiateTag			= localVerificationTag;
					initAck->advertisedReceiverWindowCredit	= localAdvertisedReceiverWindowCredit;
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
					Enqueue(std::static_pointer_cast<Chunk>(initAck));
					
					break;
				}
				case Chunk::Type::COOKIE_ECHO:
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
					Enqueue(std::static_pointer_cast<Chunk>(cookieAck));
					
					//Change state
					SetState(State::Established);
				}
				default:
					//Error
					break;
			}
			break;
		}
		case State::CookieWait:
		{
			switch(chunk->type)
			{
				case Chunk::Type::INIT_ACK:
				{
					//Get chunk of correct type
					auto initAck = std::static_pointer_cast<InitiationAcknowledgementChunk>(chunk);
					
					//	C) Upon reception of the INIT ACK from "Z", "A" shall stop the T1-
					//	init timer and leave the COOKIE-WAIT state.  "A" shall then send
					//	the State Cookie received in the INIT ACK chunk in a COOKIE ECHO
					//	chunk, start the T1-cookie timer, and enter the COOKIE-ECHOED
					//	state.
					//
					
					// Stop timer
					initTimer->Cancel();
					
					//Enqueue new INIT chunk
					auto cookieEcho = std::make_shared<CookieEchoChunk>();
					
					//Copy cookie
					cookieEcho->cookie.SetData(initAck->stateCookie);
					
					//Reset cookie retransmissions
					initRetransmissions = 0;
					
					//Set timer
					cookieEchoTimer = timeService.CreateTimer(100ms,[&](...){
						
						//3)  If the T1-cookie timer expires, the endpoint MUST retransmit
						//    COOKIE ECHO and restart the T1-cookie timer without changing
						//    state.  This MUST be repeated up to 'Max.Init.Retransmits' times.
						//    After that, the endpoint MUST abort the initialization process
						//    and report the error to the SCTP user.
						if (initRetransmissions++<MaxInitRetransmits)
						{
							//Retransmit
							Enqueue(std::static_pointer_cast<Chunk>(cookieEcho));
							//Retry again
							initTimer->Again(100ms);
						} else {
							//Close
							SetState(State::Closed);
						}
						
					});
					
					///Enquee
					Enqueue(std::static_pointer_cast<Chunk>(cookieEcho));
					
					//Set new state
					SetState(State::CookieEchoed);
				}
				default:
					//Error
					break;
			}
			break;
		}
		case State::CookieEchoed:
		{
			switch(chunk->type)
			{
				case Chunk::Type::COOKIE_ACK:
				{
					//	E) Upon reception of the COOKIE ACK, endpoint "A" will move from the
					//	COOKIE-ECHOED state to the ESTABLISHED state, stopping the T1-
					//	cookie timer.
					
					// Stop timer
					cookieEchoTimer->Cancel();
					
					//Change state
					SetState(State::Established);
				}
				default:
					//Error
					break;
			}
			break;
		}
		case State::Established:
		{
			switch(chunk->type)
			{
				case Chunk::Type::PDATA:
				{
					//	After the reception of the first DATA chunk in an association the
					//	endpoint MUST immediately respond with a SACK to acknowledge the DATA
					//	chunk.  Subsequent acknowledgements should be done as described in
					//	Section 6.2.
					break;
				}
				case Chunk::Type::SACK:
				{
					break;
				}
			}
			break;
		}
		case State::ShutdownPending:
		{
			break;
		}
		case State::ShutDownSent:
		{
			break;
		}
		case State::ShutDownReceived:
		{
			break;
		}
		case State::ShutDown:
		{
			break;
		}
		case State::ShutDownAckSent:
		{
			break;
		}
	}
}

void Association::Enqueue(const Chunk::shared& chunk)
{
	bool wasPending = pendingData;
	//Push back
	queue.push_back(chunk);
	//Reset flag
	pendingData = false;
	//If it is first
	if (!wasPending && onPendingData)
		//Call callback
		onPendingData();
}

}; //namespace sctp
