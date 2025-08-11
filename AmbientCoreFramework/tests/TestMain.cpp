#include <gtest/gtest.h>

TEST(BasicTest, GoogleTestWorks)
{
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
}

extern void hello();

TEST(BasicTest, HelloFunctionExists)
{
    EXPECT_NO_THROW(hello());
}