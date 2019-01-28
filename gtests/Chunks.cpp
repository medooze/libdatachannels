/* 
 * File:   Chunks
 * Author: Sergio
 *
 * Created on 26-ene-2019, 22:25:25
 */

#include <gtest/gtest.h>

#include "BufferReader.h"
#include "sctp/PacketHeader.h"
#include "sctp/Chunk.h"

class Chunks : public testing::Test
{
protected:
};

TEST_F(Chunks, ParseInit)
{
	uint8_t data[100] = {
	   0x13, 0x88, 0x13, 0x88,
	   0x00, 0x00, 0x00, 0x00,
	   0xdf, 0x9d, 0x5e, 0x22,
	   0x01, 0x00, 0x00, 0x56,
	   0xff, 0x55, 0x39, 0x12,
	   0x00, 0x02, 0x00, 0x00,
	   0x04, 0x00, 0x08, 0x00,
	   0xce, 0x91, 0x30, 0x32,
	   0xc0, 0x00, 0x00, 0x04,
	   0x80, 0x08, 0x00, 0x09,
	   0xc0, 0x0f, 0xc1, 0x80,
	   0x82, 0x00, 0x00, 0x00,
	   0x80, 0x02, 0x00, 0x24,
	   0xf1, 0x77, 0x00, 0x00,
	   0x51, 0x0a, 0x00, 0x00,
	   0xba, 0x21, 0x00, 0x00,
	   0xd2, 0x39, 0x00, 0x00,
	   0xc2, 0x38, 0x00, 0x00,
	   0xbb, 0x7f, 0x00, 0x00,
	   0x25, 0x53, 0x00, 0x00,
	   0xd8, 0x44, 0x00, 0x00,
	   0x80, 0x04, 0x00, 0x06,
	   0x00, 0x01, 0x00, 0x00,
	   0x80, 0x03, 0x00, 0x06,
	};
	
	BufferReader reader(data,100);
	
	auto header = sctp::PacketHeader::Parse(reader);
	ASSERT_TRUE(header);
	auto chunk = sctp::Chunk::Parse(reader);
	ASSERT_TRUE(chunk);
	ASSERT_EQ(chunk->type,sctp::Chunk::INIT);
	ASSERT_FALSE(reader.GetLeft());

}

TEST_F(Chunks, SerializeInit)
{
	Buffer buffer(1200);
	
	//Create init chunk
	sctp::InitiationChunk init;
	init.advertisedReceiverWindowCredit = 6500;
	init.numberOfOutboundStreams = 1024;
	init.numberOfInboundStreams = 2048;
	init.initialTransmissionSequenceNumber = 128;
	init.suggestedCookieLifeSpanIncrement = 32000;
	init.supportedAddressTypes = {1,2};
	init.supportedExtensions = {3,4,5,6,7};
	init.forwardTSNSupported = true;
	
	//Serialize
	BufferWritter writter(buffer);
	size_t len = init.Serialize(writter);
	ASSERT_TRUE(len);
	buffer.SetSize(len);

	//Parse it again
	BufferReader reader(buffer);
	auto chunk = sctp::Chunk::Parse(reader);
	ASSERT_TRUE(chunk);
	ASSERT_EQ(chunk->type,sctp::Chunk::INIT);
	//Check chunk is equal to init chunk
	auto init2 = std::static_pointer_cast<sctp::InitiationChunk>(chunk);
	ASSERT_EQ(init2->advertisedReceiverWindowCredit		,init.advertisedReceiverWindowCredit);
	ASSERT_EQ(init2->numberOfOutboundStreams		,init.numberOfOutboundStreams);
	ASSERT_EQ(init2->numberOfInboundStreams			,init.numberOfInboundStreams);
	ASSERT_EQ(init2->initialTransmissionSequenceNumber	,init.initialTransmissionSequenceNumber);
	ASSERT_EQ(init2->suggestedCookieLifeSpanIncrement	,init.suggestedCookieLifeSpanIncrement);
	ASSERT_EQ(init2->supportedAddressTypes.size()		,init.supportedAddressTypes.size());
	ASSERT_EQ(init2->supportedExtensions.size()		,init.supportedExtensions.size());
	ASSERT_EQ(init2->forwardTSNSupported			,init.forwardTSNSupported);
	
}
