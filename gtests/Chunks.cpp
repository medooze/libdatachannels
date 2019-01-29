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
	
	//Create chunk
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

TEST_F(Chunks, SerializeInitAck)
{
	Buffer buffer(1200);
	uint8_t cookie[5] = {0,1,2,3,4};
	
	//Create chunk
	sctp::InitiationAcknowledgementChunk ack;
	ack.advertisedReceiverWindowCredit = 6500;
	ack.numberOfOutboundStreams = 1024;
	ack.numberOfInboundStreams = 2048;
	ack.initialTransmissionSequenceNumber = 128;
	ack.supportedExtensions = {3,4,5,6,7};
	ack.stateCookie.SetData(cookie,sizeof(cookie));
	ack.forwardTSNSupported = true;
	
	//Serialize
	BufferWritter writter(buffer);
	size_t len = ack.Serialize(writter);
	ASSERT_TRUE(len);
	buffer.SetSize(len);

	//Parse it again
	BufferReader reader(buffer);
	auto chunk = sctp::Chunk::Parse(reader);
	ASSERT_TRUE(chunk);
	ASSERT_EQ(chunk->type,sctp::Chunk::INIT_ACK);
	//Check chunk is equal to init chunk
	auto ack2 = std::static_pointer_cast<sctp::InitiationAcknowledgementChunk>(chunk);
	ASSERT_EQ(ack2->advertisedReceiverWindowCredit		,ack.advertisedReceiverWindowCredit);
	ASSERT_EQ(ack2->numberOfOutboundStreams			,ack.numberOfOutboundStreams);
	ASSERT_EQ(ack2->numberOfInboundStreams			,ack.numberOfInboundStreams);
	ASSERT_EQ(ack2->initialTransmissionSequenceNumber	,ack.initialTransmissionSequenceNumber);
	ASSERT_EQ(ack2->stateCookie.GetSize()			,ack.stateCookie.GetSize());
	ASSERT_EQ(ack2->supportedExtensions.size()		,ack.supportedExtensions.size());
	ASSERT_EQ(ack2->forwardTSNSupported			,ack.forwardTSNSupported);
	
}


TEST_F(Chunks, SerializeCookieEcho)
{
	Buffer buffer(1200);
	uint8_t cookie[5] = {0,1,2,3,4};
	
	//Create chunk
	sctp::CookieEchoChunk echo;
	echo.cookie.SetData(cookie,sizeof(cookie));
	
	//Serialize
	BufferWritter writter(buffer);
	size_t len = echo.Serialize(writter);
	ASSERT_TRUE(len);
	buffer.SetSize(len);

	//Parse it again
	BufferReader reader(buffer);
	auto chunk = sctp::Chunk::Parse(reader);
	ASSERT_TRUE(chunk);
	ASSERT_EQ(chunk->type,sctp::Chunk::COOKIE_ECHO);
	//Check chunk is equal to init chunk
	auto echo2 = std::static_pointer_cast<sctp::CookieEchoChunk>(chunk);
	ASSERT_EQ(echo2->cookie.GetSize()			,echo.cookie.GetSize());
}


TEST_F(Chunks, SerializeCookieAck)
{
	Buffer buffer(1200);
	//Create chunk
	sctp::CookieAckChunk ack;
	
	//Serialize
	BufferWritter writter(buffer);
	size_t len = ack.Serialize(writter);
	ASSERT_TRUE(len);
	buffer.SetSize(len);

	//Parse it again
	BufferReader reader(buffer);
	auto chunk = sctp::Chunk::Parse(reader);
	ASSERT_TRUE(chunk);
	ASSERT_EQ(chunk->type,sctp::Chunk::COOKIE_ACK);
	//Check chunk is equal to init chunk
	auto ack2 = std::static_pointer_cast<sctp::CookieAckChunk>(chunk);
	ASSERT_TRUE(ack2);
	
}

TEST_F(Chunks, SerializeHeartbeatRequest)
{
	Buffer buffer(1200);
	uint8_t info[5] = {0,1,2,3,4};
	
	//Create chunk
	sctp::HeartbeatRequestChunk heartbeat;
	heartbeat.senderSpecificHearbeatInfo.SetData(info,sizeof(info));
	
	//Serialize
	BufferWritter writter(buffer);
	size_t len = heartbeat.Serialize(writter);
	ASSERT_TRUE(len);
	buffer.SetSize(len);

	//Parse it again
	BufferReader reader(buffer);
	auto chunk = sctp::Chunk::Parse(reader);
	ASSERT_TRUE(chunk);
	ASSERT_EQ(chunk->type,sctp::Chunk::HEARTBEAT);
	//Check chunk is equal to init chunk
	auto heartbeat2 = std::static_pointer_cast<sctp::HeartbeatRequestChunk>(chunk);
	ASSERT_EQ(heartbeat2->senderSpecificHearbeatInfo.GetSize()	,heartbeat.senderSpecificHearbeatInfo.GetSize());
}

TEST_F(Chunks, SerializeHeartbeatAck)
{
	Buffer buffer(1200);
	uint8_t info[5] = {0,1,2,3,4};
	
	//Create chunk
	sctp::HeartbeatAckChunk heartbeat;
	heartbeat.senderSpecificHearbeatInfo.SetData(info,sizeof(info));
	
	//Serialize
	BufferWritter writter(buffer);
	size_t len = heartbeat.Serialize(writter);
	ASSERT_TRUE(len);
	buffer.SetSize(len);

	//Parse it again
	BufferReader reader(buffer);
	auto chunk = sctp::Chunk::Parse(reader);
	ASSERT_TRUE(chunk);
	ASSERT_EQ(chunk->type,sctp::Chunk::HEARTBEAT_ACK);
	//Check chunk is equal to init chunk
	auto heartbeat2 = std::static_pointer_cast<sctp::HeartbeatAckChunk>(chunk);
	ASSERT_EQ(heartbeat2->senderSpecificHearbeatInfo.GetSize()	,heartbeat.senderSpecificHearbeatInfo.GetSize());
}

TEST_F(Chunks, SerializePadding)
{
	Buffer buffer(1200);
	uint8_t info[7] = {0,1,2,3,4,5,6};
	
	//Create chunk
	sctp::PaddingChunk padding;
	padding.buffer.SetData(info,sizeof(info));
	
	//Serialize
	BufferWritter writter(buffer);
	size_t len = padding.Serialize(writter);
	ASSERT_TRUE(len);
	buffer.SetSize(len);

	//Parse it again
	BufferReader reader(buffer);
	auto chunk = sctp::Chunk::Parse(reader);
	ASSERT_TRUE(chunk);
	ASSERT_EQ(chunk->type,sctp::Chunk::PAD);
	//Check chunk is equal to init chunk
	auto padding2 = std::static_pointer_cast<sctp::PaddingChunk>(chunk);
	ASSERT_EQ(padding2->buffer.GetSize()		,padding.buffer.GetSize());
}

TEST_F(Chunks, SerializeUnknown)
{
	Buffer buffer(1200);
	uint8_t info[8] = {0,1,2,3,4,5,6,7};
	
	//Create chunk
	sctp::UnknownChunk unknown(255);
	unknown.buffer.SetData(info,sizeof(info));
	
	//Serialize
	BufferWritter writter(buffer);
	size_t len = unknown.Serialize(writter);
	ASSERT_TRUE(len);
	buffer.SetSize(len);

	//Parse it again
	BufferReader reader(buffer);
	auto chunk = sctp::Chunk::Parse(reader);
	ASSERT_TRUE(chunk);
	ASSERT_EQ(chunk->type,255);
	//Check chunk is equal to init chunk
	auto unknown2 = std::static_pointer_cast<sctp::UnknownChunk>(chunk);
	ASSERT_EQ(unknown2->buffer.GetSize()		,unknown.buffer.GetSize());
}
