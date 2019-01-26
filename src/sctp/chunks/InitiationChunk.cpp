#include "sctp/chunks/InitiationChunk.h"

namespace sctp
{
	
size_t InitiationChunk::GetSize() const
{
	//Header + attributes
	size_t size = 20;
	
	//Set parameters
	size += ipV4Addresses.size() * 8;
	size += ipV6Addresses.size() * 20;
	if (suggestedCookieLifeSpanIncrement)
		size += 8;
	if (hostName)
		size += SizePad(hostName->length(), 4) ;
	size += SizePad(supportedAddressTypes.size() * 2, 4);
	size += SizePad(supportedExtensions.size(), 4);
	for (const auto& unknownParameter : unknownParameters)
		size += SizePad(unknownParameter.second.GetSize(), 4);
	
	//Done
	return size;
}

size_t InitiationChunk::Serialize(BufferWritter& writter) const
{
	//Write header
	writter.Set1(type);
	writter.Set1(0);
	//Skip length position
	size_t mark = writter.Skip(2);
	
	//Set attributes
	writter.Set4(initiateTag);
	writter.Set4(advertisedReceiverWindowCredit);
	writter.Set2(numberOfOutboundStreams);
	writter.Set2(numberOfInboundStreams);
	writter.Set4(initialTransmissionSequenceNumber);
	
	for (const auto& ipV4Address : ipV4Addresses)
	{
		//Write it
		writter.Set2(Parameter::IPv4Address);
		writter.Set2(8+4);
		writter.Set<8>(ipV4Address);
		writter.PadTo(4);
	}
	for (const auto& ipV6Address : ipV6Addresses)
	{
		//Write it
		writter.Set2(Parameter::IPv6Address);
		writter.Set2(20+4);
		writter.Set<20>(ipV6Address);
		writter.PadTo(4);
	}
	if (suggestedCookieLifeSpanIncrement)
	{
		//Write it
		writter.Set2(Parameter::CookiePreservative);
		writter.Set2(8+4);
		writter.Set8(*suggestedCookieLifeSpanIncrement);
		writter.PadTo(4);
	}
	if (hostName)
	{
		//Write it
		writter.Set2(Parameter::HostNameAddress);
		writter.Set2(hostName->length()+4);
		writter.Set(*hostName);
		writter.PadTo(4);
	}
	if (supportedAddressTypes.size())
	{
		//Write it
		writter.Set2(Parameter::SupportedAddressTypes);
		writter.Set2(supportedAddressTypes.size()*2+4);
		for (const auto& supportedAddressType : supportedAddressTypes)
			writter.Set2(supportedAddressType);
		writter.PadTo(4);
	}
	if (supportedExtensions.size())
	{
		//Write it
		writter.Set2(Parameter::SupportedExtensions);
		writter.Set2(supportedExtensions.size()+4);
		for (const auto& supportedExtension : supportedExtensions)
			writter.Set1(supportedExtension);
		writter.PadTo(4);
	}
	for (const auto& unknownParameter : unknownParameters)
	{
		//Write it
		writter.Set2(unknownParameter.first);
		writter.Set2(unknownParameter.second.GetSize()+4);
		writter.Set(unknownParameter.second);
		writter.PadTo(4);
	}
	if (forwardTSNSupported)
	{
		//Write it
		writter.Set2(Parameter::ForwardTSNSupported);
		writter.Set2(4);
		writter.PadTo(4);
	}
	
	//Get length
	size_t length = writter.GetLength();
	//Set it
	writter.Set2(mark,length);
	
	//Done
	return length;
}
	
Chunk::shared InitiationChunk::Parse(BufferReader& reader)
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
	if (type!=Type::INIT)
		//Error
		return nullptr;
		
	//Create chunk
	auto init = std::make_shared<InitiationChunk>();
		
	//Set attributes
	init->initiateTag			= reader.Get4();
	init->advertisedReceiverWindowCredit	= reader.Get4();
	init->numberOfOutboundStreams		= reader.Get2();
	init->numberOfInboundStreams		= reader.Get2();
	init->initialTransmissionSequenceNumber = reader.Get4();
	init->forwardTSNSupported		= false;
	
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
		if (!reader.Assert(paramLength-4)) return nullptr;
		//Get reader for the param length
		BufferReader paramReader = reader.GetReader(paramLength-4);
		//Depending on the parameter type
		switch(paramType)
		{
			case Parameter::IPv4Address:
				if (!paramReader.Assert(8)) return nullptr;
				init->ipV4Addresses.push_back(paramReader.Get<8>());
				break;
			case Parameter::IPv6Address:
				if (!paramReader.Assert(20)) return nullptr;
				init->ipV6Addresses.push_back(paramReader.Get<20>());
				break;
			case Parameter::HostNameAddress:
				init->hostName = paramReader.GetString(paramReader.GetLeft());
				break;
			case Parameter::SupportedAddressTypes:
				while (paramReader.GetLeft())
					init->supportedAddressTypes.push_back(paramReader.Get2());
				break;
			case Parameter::SupportedExtensions:
				while (paramReader.GetLeft())
					init->supportedExtensions.push_back(paramReader.Get1());
				break;
			case Parameter::CookiePreservative:
				if (!paramReader.Assert(8)) return nullptr;
				init->suggestedCookieLifeSpanIncrement = paramReader.Get8();
				break;
			case Parameter::ForwardTSNSupported:
				init->forwardTSNSupported = true;
				break;
			default:
				//Unkonwn
				init->unknownParameters.push_back(std::make_pair<uint8_t,Buffer>(paramType,paramReader.GetBuffer(paramReader.GetLeft())));
		}
		//Ensure all input has been consumed
		if (paramReader.GetLeft())
			throw new std::runtime_error("Wrong parameter");
		//Do padding
		reader.PadTo(4);
	}
	
	//Done
	return std::static_pointer_cast<Chunk>(init);
}
	
};
