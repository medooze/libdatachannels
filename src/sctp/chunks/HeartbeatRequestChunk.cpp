#include "sctp/chunks/InitiationChunk.h"

namespace sctp
{
	
size_t InitiationChunk::GetSize()
{
	//Header + attributes
	size_t size = 20;
	
	//Done
	return size;
}

size_t InitiationChunk::Serialize(BuffferWritter& writter)
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
	
Chunk::Shared InitiationChunk::Parse(const BuffferReader& reader)
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
		
	//Done
	return std::static_pointer_cast<Chunk>(init);
}
	
};

