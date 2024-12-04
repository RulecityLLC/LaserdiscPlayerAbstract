#ifndef LDP_ABST_STDAFX_H
#define LDP_ABST_STDAFX_H

// Compiling tests may go faster if we use pre-compiled headers for these massive google test includes
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Test compatibility macros

#define TEST_CHECK(a)   EXPECT_TRUE(a)

#define TEST_REQUIRE(a) ASSERT_TRUE(a)

#define TEST_CHECK_EQUAL(a,b)   EXPECT_EQ(a,b)

#define TEST_REQUIRE_EQUAL(a,b) ASSERT_EQ(a,b)

#define TEST_CHECK_NOT_EQUAL(a,b)   EXPECT_NE(a,b)

#define TEST_REQUIRE_NOT_EQUAL(a,b) ASSERT_NE(a,b)

#define TEST_CHECK_THROW(a) { bool bCaught = false; try { a; } catch (...) { bCaught = true; } EXPECT_TRUE(bCaught); }

// normal test case (all tests will run)
// TEST is from google test
#define TEST_CASE(name) TEST(LdpAbstTest, name)

// exclusive test flag ignored for now
#define TEST_CASE_EX(name) TEST_CASE(name)

#endif //LDP_ABST_STDAFX_H
