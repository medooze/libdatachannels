/* 
 * File:   Association
 * Author: Sergio
 *
 * Created on 30-ene-2019, 17:47:50
 */

#include <gtest/gtest.h>

#include "Buffer.h"
#include "FakeTimeService.h"
#include "sctp/Association.h"

class Association : public testing::Test
{
protected:
};


TEST_F(Association, Init)
{
	FakeTimeService timeService;
	sctp::Association association(timeService);
	
	association.SetLocalPort(1000);
	association.SetRemotePort(2000);
	
	ASSERT_EQ(association.GetState(),sctp::Association::Closed);
	ASSERT_EQ(association.HasPendingData(),false);
}

TEST_F(Association, EmptyRead)
{
	Buffer buffer(1500);
	FakeTimeService timeService;
	sctp::Association association(timeService);
	ASSERT_EQ(association.HasPendingData(),false);
	ASSERT_FALSE(association.ReadPacket(buffer));
}

TEST_F(Association, Associate)
{
	FakeTimeService timeService;
	sctp::Association association(timeService);
	
	association.SetLocalPort(1000);
	association.SetRemotePort(2000);
	
	//Start association
	association.Associate();
	
	//Init should be rtx three times
	size_t rtx = 0;
	//Ensure retransmissions
	while (rtx++<=sctp::Association::MaxInitRetransmits)
	{
		ASSERT_EQ(association.GetState(),sctp::Association::CookieWait);
		ASSERT_EQ(association.HasPendingData(),true);
	
		Buffer buffer(1500);
		//Read data
		size_t len = association.ReadPacket(buffer);
		//Ensure size
		ASSERT_TRUE(len);
		
		BufferReader reader(buffer);
		
		//Parse header
		auto header = sctp::PacketHeader::Parse(reader);
		
		//Ensure everything is correct
		ASSERT_EQ(header->sourcePortNumber	, association.GetLocalPort());
		ASSERT_EQ(header->destinationPortNumber	, association.GetRemotePort());
		ASSERT_EQ(header->verificationTag	, 0);
		ASSERT_TRUE(header->checksum);
		
		//Read chunk
		auto chunk = sctp::Chunk::Parse(reader);
		ASSERT_TRUE(chunk);
		ASSERT_EQ(chunk->type,sctp::Chunk::INIT);
		
		//Ensure state
		ASSERT_EQ(association.GetState(),sctp::Association::CookieWait);
		ASSERT_EQ(association.HasPendingData(),false);
		
		//Launch next retransmission
		timeService.SetNow(timeService.GetNow() + sctp::Association::InitRetransmitTimeout);
	}
	//Ensure state
	ASSERT_EQ(association.GetState(),sctp::Association::Closed);
	ASSERT_EQ(association.HasPendingData(),false);
}

