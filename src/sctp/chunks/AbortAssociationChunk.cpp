#include "sctp/chunks/AbortAssociationChunk.h"

namespace sctp
{
	
size_t AbortAssociationChunk::GetSize() const
{
	//Header + attributes
	size_t size = 20;
	
	//Done
	return size;
}

size_t AbortAssociationChunk::Serialize(BufferWritter& writter) const
{
	//Get init pos
	size_t ini = writter.Mark();
	
	//Write header
	writter.Set1(type);
	writter.Set1(0);
	//Skip length position
	size_t mark = writter.Skip(2);
	
	//Get length
	size_t length = writter.GetOffset(ini);
	//Set it
	writter.Set2(mark,length);
	
	//Done
	return length;
}
	
Chunk::shared AbortAssociationChunk::Parse(BufferReader& reader)
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
	
	(void)mark;
	(void)flag;
	(void)length;
	
	//Check type
	if (type!=Type::ABORT)
		//Error
		return nullptr;
		
	//Create chunk
	auto abort = std::make_shared<AbortAssociationChunk>();
		
	//Done
	return std::static_pointer_cast<Chunk>(abort);
}
	
};
