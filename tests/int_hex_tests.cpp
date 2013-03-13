#include <gtest/gtest.h>

#include <limits>

#include "../bf/inthex.h"

using namespace bitforge;

TEST(IntStr, TestUIntStr)
{
    const int bufferSize = 48;
    char buffer[bufferSize];
    
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 9), "9");
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 19), "19");
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 1000000), "1000000");
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 1234567890), "1234567890");
}

TEST(IntStr, TestIntStr)
{
    const int bufferSize = 48;
    char buffer[bufferSize];
    
    ASSERT_STREQ(inttostr(buffer, bufferSize, 9), "9");
    ASSERT_STREQ(inttostr(buffer, bufferSize, 19), "19");
    ASSERT_STREQ(inttostr(buffer, bufferSize, 1000000), "1000000");
    ASSERT_STREQ(inttostr(buffer, bufferSize, 1234567890), "1234567890");
    
    ASSERT_STREQ(inttostr(buffer, bufferSize, -9), "-9");
    ASSERT_STREQ(inttostr(buffer, bufferSize, -19), "-19");
    ASSERT_STREQ(inttostr(buffer, bufferSize, -1000000), "-1000000");
    ASSERT_STREQ(inttostr(buffer, bufferSize, -1234567890), "-1234567890");
}

TEST(IntStr, TestUIntStrBases)
{
    const int bufferSize = 48;
    char buffer[bufferSize];
    
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 9, 8), "11");
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 19, 8), "23");
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 1000000, 8), "3641100");
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 1234567890, 8), "11145401322");
    
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 9, 2), "1001");
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 19, 2), "10011");
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 1000000, 2), "11110100001001000000");
    ASSERT_STREQ(uinttostr(buffer, bufferSize, 1234567890, 2), "1001001100101100000001011010010");
}

TEST(IntStr, TestUInt64Str)
{
    const uint64_t max32 = 2147483647ll;
    
    const int bufferSize = 128;
    char buffer[bufferSize];
    
    ASSERT_STREQ(uint64tostr(buffer, bufferSize, 9), "9");
    ASSERT_STREQ(uint64tostr(buffer, bufferSize, 19), "19");
    ASSERT_STREQ(uint64tostr(buffer, bufferSize, 1000000), "1000000");
    ASSERT_STREQ(uint64tostr(buffer, bufferSize, 1234567890), "1234567890");
    
    ASSERT_STREQ(uint64tostr(buffer, bufferSize, max32 + 9), "2147483656");
    ASSERT_STREQ(uint64tostr(buffer, bufferSize, max32 + 19), "2147483666");
    ASSERT_STREQ(uint64tostr(buffer, bufferSize, max32 + 1000000), "2148483647");
    ASSERT_STREQ(uint64tostr(buffer, bufferSize, max32 + 1234567890), "3382051537");
}

TEST(IntStr, TestInt64Str)
{
    const int64_t max32 = 2147483647;
    const int64_t min32 = -2147483646;
    
    const int bufferSize = 48;
    char buffer[bufferSize];
    
    ASSERT_STREQ(int64tostr(buffer, bufferSize, 9), "9");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, 19), "19");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, 1000000), "1000000");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, 1234567890), "1234567890");
    
    ASSERT_STREQ(int64tostr(buffer, bufferSize, -9), "-9");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, -19), "-19");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, -1000000), "-1000000");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, -1234567890), "-1234567890");
    
    ASSERT_STREQ(int64tostr(buffer, bufferSize, max32 + 9), "2147483656");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, max32 + 19), "2147483666");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, max32 + 1000000), "2148483647");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, max32 + 1234567890), "3382051537");
    
    ASSERT_STREQ(int64tostr(buffer, bufferSize, min32 - 9ll), "-2147483655");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, min32 - 19ll), "-2147483665");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, min32 - 1000000ll), "-2148483646");
    ASSERT_STREQ(int64tostr(buffer, bufferSize, min32 - 1234567890ll), "-3382051536");
}


TEST(IntStr, TestHexToInt)
{   
    ASSERT_EQ(hextoint("9"), 9);
    ASSERT_EQ(hextoint("19"), 25);
    ASSERT_EQ(hextoint("10ACE0f"), 17485327);
    ASSERT_EQ(hextoint("499602D2"), 1234567890);
}

TEST(IntStr, TestHexToInt64)
{   
    ASSERT_EQ(hextoint64("80000008"), 2147483656ll);
    ASSERT_EQ(hextoint64("80000012"), 2147483666ll);
    ASSERT_EQ(hextoint64("800F423F"), 2148483647ll);
    ASSERT_EQ(hextoint64("C99602D1"), 3382051537ll);
}

TEST(IntStr, TestUIntToHex)
{   
    const int bufferSize = 48;
    char buffer[bufferSize];
    
    ASSERT_STREQ(uinttohex(buffer, bufferSize, 9), "9");
    ASSERT_STREQ(uinttohex(buffer, bufferSize, 25), "19");
    ASSERT_STREQ(uinttohex(buffer, bufferSize, 17485327), "10ACE0F");
    ASSERT_STREQ(uinttohex(buffer, bufferSize, 1234567890), "499602D2");
}
