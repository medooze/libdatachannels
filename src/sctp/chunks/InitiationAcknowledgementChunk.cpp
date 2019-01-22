#include "sctp/chunks/InitiationAcknowledgementChunk.h"

namespace sctp
{
	
size_t InitiationAcknowledgementChunk::GetSize() const
{
	//Header + attributes
	size_t size = 20;
	
	//Done
	return size;
}

size_t InitiationAcknowledgementChunk::Serialize(BufferWritter& writter) const
{
	//Write header
	writter.Set1(type);
	writter.Set1(0);
	//Skip length position
	size_t mark = writter.Skip(2);
	
	
	
	//Get length
	size_t length = writter.GetLength();
	//Set it
	writter.Set2(mark,length);
	
	//Done
	return length;
}
	
Chunk::shared InitiationAcknowledgementChunk::Parse(BufferReader& reader)
{
	//Check size
	if (!reader.Assert(20)) 
		//Error
		return nullptr;
	
	//Get header
	size_t mark	= reader.Mark();
	uint8_t type	= reader.Get1();
	uint8_t flag	= reader.Get1(); //Ignored, should be 0
	uint16_t length	= reader.Get2();
	
	//Check type
	if (type!=Type::INIT_ACK)
		//Error
		return nullptr;
	
	//Create chunk
	auto ack = std::make_shared<InitiationAcknowledgementChunk>();
	
	//Set attributes
	ack->initiateTag			= reader.Get4();
	ack->advertisedReceiverWindowCredit	= reader.Get4();
	ack->numberOfOutboundStreams		= reader.Get2();
	ack->numberOfInboundStreams		= reader.Get2();
	ack->initialTransmissionSequenceNumber	= reader.Get4();
	ack->forwardTSNSupported		= false;
	
	//Read parameters
	while (reader.GetLeft()>4)
	{
		//Get parameter type
		uint16_t paramType = reader.Get2();
		uint16_t paramLength = reader.Get2();
		//Ensure lenghth is correct as it has to contain the type and length itself
		if (paramLength<4)
			throw new std::runtime_error("Wrong parameter length");
		//Ensure we have enought length
		if (!reader.Assert(paramLength)) return nullptr;
		//Get reader for the param length
		BufferReader paramReader = reader.GetReader(paramLength-4);
		//Depending on the parameter type
		switch(paramType)
		{
			case Parameter::IPv4Address:
				if (!paramReader.Assert(8)) return nullptr;
				ack->ipV4Addresses.push_back(paramReader.Get<8>());
				break;
			case Parameter::IPv6Address:
				if (!paramReader.Assert(20)) return nullptr;
				ack->ipV6Addresses.push_back(paramReader.Get<20>());
				break;
			case Parameter::HostNameAddress:
				ack->hostName = paramReader.GetString(paramReader.GetLeft());
				break;
			case Parameter::StateCookie:
				ack->stateCookie = paramReader.GetBuffer(paramReader.GetLeft());
				break;
			case Parameter::SupportedExtensions:
				for (size_t i=0; i<paramLength; ++i)
					ack->supportedExtensions.push_back(paramReader.Get1());
				break;
			case Parameter::ForwardTSNSupported:
				ack->forwardTSNSupported = true;
				break;
			case Parameter::UnrecognizedParameter:
				ack->unrecognizedParameters.push_back(paramReader.GetBuffer(paramReader.GetLeft()));
				break;
			default:
				//Unkonwn
				ack->unknownParameters.push_back(std::make_pair<uint8_t,Buffer>(paramType,paramReader.GetBuffer(paramReader.GetLeft())));
		}
		//Ensure all input has been consumed
		if (paramReader.GetLeft())
			throw new std::runtime_error("Wrong parameter");
		//Do padding
		reader.PadTo(4);
	}
	
	//Done
	return std::static_pointer_cast<Chunk>(ack);
}
	
};
