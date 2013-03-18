#include <gtest/gtest.h>

#include <limits>

#include "../bf/bf.h"

using namespace bitforge;

TEST(Util, TestFletcher32)
{
    const char testData[] = "Hello World!";

    ASSERT_EQ(fletcher32(testData, sizeof(testData)), 1205661521);
}
