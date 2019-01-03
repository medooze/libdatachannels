/* 
 * File:   TestSuite
 * Author: Sergio
 *
 * Created on 30-dic-2018, 22:48:55
 */

#include <gtest/gtest.h>

class TestSuite : public testing::Test
{
protected:

	void SetUp() {
		// Setup ...
	}

	void TearDown() {
		// Teardown ...
	}

};

TEST_F(TestSuite, testExample)
{
	FAIL();
}

