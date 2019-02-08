/* 
 * File:   Association
 * Author: Sergio
 *
 * Created on 30-ene-2019, 17:47:50
 */

#include <gtest/gtest.h>

#include "Buffer.h"
#include "sctp/SequenceNumberWrapper.h"

class SequenceNumberWrapper : public testing::Test
{
protected:
};


TEST_F(SequenceNumberWrapper, Base)
{
	
	sctp::SequenceNumberWrapper<uint32_t> wrapper;
	
	for (uint32_t seq = 0; seq<~static_cast<uint32_t>(0);seq+=0xFFF)
	{
		auto wrapped   = wrapper.Wrap(seq);
		auto unwrapped = wrapper.Wrap(wrapped);
		ASSERT_EQ(wrapped,unwrapped);
	}
}

TEST_F(SequenceNumberWrapper, Wrap)
{
	
	sctp::SequenceNumberWrapper<uint32_t> wrapper;
	
	//Last seq number in wrap
	auto last  = wrapper.Wrap(~0);
	auto next  = wrapper.Wrap(0);
	//Check extended numbers are consecutive
	ASSERT_LT(last,next);
	ASSERT_EQ(next-last,1);
	ASSERT_EQ(~0,wrapper.UnWrap(last));
	ASSERT_EQ(0 ,wrapper.UnWrap(next));
	
}

TEST_F(SequenceNumberWrapper, OutOfOrder)
{

	sctp::SequenceNumberWrapper<uint32_t> wrapper;
	
	auto one  = wrapper.Wrap(1);
	auto zero   = wrapper.Wrap(0);
	//Check extended numbers are consecutive
	ASSERT_LT(zero,one);
	ASSERT_EQ(one-zero,1);
	ASSERT_EQ(0, wrapper.UnWrap(zero));
	ASSERT_EQ(one, wrapper.UnWrap(one));
	
}

TEST_F(SequenceNumberWrapper, PrevWrap)
{
	
	sctp::SequenceNumberWrapper<uint32_t> wrapper;
	
	//Last seq number in wrap
	auto last  = wrapper.Wrap(~0);
	auto next  = wrapper.Wrap(0);
	auto ooo   = wrapper.Wrap(~0-1);
	//Check extended numbers are consecutive
	ASSERT_LT(last,next);
	ASSERT_LT(ooo,last);
	ASSERT_LT(ooo,next);
	ASSERT_EQ(next-last,1);
	ASSERT_EQ(next-ooo,2);
	ASSERT_EQ(last-ooo,1);
	ASSERT_EQ(~0,wrapper.UnWrap(last));
	ASSERT_EQ(0 ,wrapper.UnWrap(next));
	ASSERT_EQ(~0-1,wrapper.UnWrap(ooo));
	
}
