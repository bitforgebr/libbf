#include <gtest/gtest.h>

#include <limits>

#include "../bf/bf.h"

using namespace bitforge;

TEST(Util, TestFletcher32)
{
    const char testData[] = "Hello World!";

    ASSERT_EQ(fletcher32(testData, sizeof(testData)), 1205661521);
}

TEST(Util, strKey)
{
    const char *test0 = "";
    const char *test1 = "H";
    const char *test2 = "He";
    const char *test3 = "Hel";
    const char *test4 = "Hell";
    const char *test5 = "Hello";

    ASSERT_EQ(strKey(test0), 0);

    ASSERT_GT(strKey(test1), 0);
    ASSERT_NE(strKey(test1), strKey(test0));

    ASSERT_GT(strKey(test2), 0);
    ASSERT_NE(strKey(test2), strKey(test0));
    ASSERT_NE(strKey(test2), strKey(test1));

    ASSERT_GT(strKey(test3), 0);
    ASSERT_NE(strKey(test3), strKey(test0));
    ASSERT_NE(strKey(test3), strKey(test1));
    ASSERT_NE(strKey(test3), strKey(test2));

    ASSERT_GT(strKey(test4), 0);
    ASSERT_NE(strKey(test4), strKey(test0));
    ASSERT_NE(strKey(test4), strKey(test1));
    ASSERT_NE(strKey(test4), strKey(test2));
    ASSERT_NE(strKey(test4), strKey(test3));

    ASSERT_GT(strKey(test5), 0);
    ASSERT_NE(strKey(test5), strKey(test0));
    ASSERT_NE(strKey(test5), strKey(test1));
    ASSERT_NE(strKey(test5), strKey(test2));
    ASSERT_NE(strKey(test5), strKey(test3));

    // This is equal since strKey can only test the first 4 chars
    ASSERT_EQ(strKey(test5), strKey(test4));
}

TEST(Util, striKey)
{
    const char *test0 = "";
    const char *test1 = "H";
    const char *test2 = "He";
    const char *test3 = "Hel";
    const char *test4 = "Hell";
    const char *test5 = "Hello";

    const char *itest1 = "h";
    const char *itest2 = "he";
    const char *itest3 = "hel";
    const char *itest4 = "hell";
    const char *itest5 = "hello";

    ASSERT_EQ(striKey(test0), 0);

    ASSERT_GT(striKey(test1), 0);
    ASSERT_NE(striKey(test1), striKey(test0));

    ASSERT_GT(striKey(test2), 0);
    ASSERT_NE(striKey(test2), striKey(test0));
    ASSERT_NE(striKey(test2), striKey(test1));

    ASSERT_GT(striKey(test3), 0);
    ASSERT_NE(striKey(test3), striKey(test0));
    ASSERT_NE(striKey(test3), striKey(test1));
    ASSERT_NE(striKey(test3), striKey(test2));

    ASSERT_GT(striKey(test4), 0);
    ASSERT_NE(striKey(test4), striKey(test0));
    ASSERT_NE(striKey(test4), striKey(test1));
    ASSERT_NE(striKey(test4), striKey(test2));
    ASSERT_NE(striKey(test4), striKey(test3));

    ASSERT_GT(striKey(test5), 0);
    ASSERT_NE(striKey(test5), striKey(test0));
    ASSERT_NE(striKey(test5), striKey(test1));
    ASSERT_NE(striKey(test5), striKey(test2));
    ASSERT_NE(striKey(test5), striKey(test3));

    // This is equal since striKey can only test the first 4 chars
    ASSERT_EQ(striKey(test5), striKey(test4));

    ASSERT_EQ(striKey(test0), strKey(test0));
    ASSERT_EQ(striKey(test1), strKey(itest1));
    ASSERT_EQ(striKey(test2), strKey(itest2));
    ASSERT_EQ(striKey(test3), strKey(itest3));
    ASSERT_EQ(striKey(test4), strKey(itest4));
    ASSERT_EQ(striKey(test5), strKey(itest5));
}

TEST(Util, _key)
{
    auto test0 = ""_key;
    auto test1 = "H"_key;
    auto test2 = "He"_key;
    auto test3 = "Hel"_key;
    auto test4 = "Hell"_key;
    auto test5 = "Hello"_key;

    ASSERT_EQ(test0, 0);

    ASSERT_GT(test1, 0);
    ASSERT_NE(test1, test0);

    ASSERT_GT(test2, 0);
    ASSERT_NE(test2, test0);
    ASSERT_NE(test2, test1);

    ASSERT_GT(test3, 0);
    ASSERT_NE(test3, test0);
    ASSERT_NE(test3, test1);
    ASSERT_NE(test3, test2);

    ASSERT_GT(test4, 0);
    ASSERT_NE(test4, test0);
    ASSERT_NE(test4, test1);
    ASSERT_NE(test4, test2);
    ASSERT_NE(test4, test3);

    ASSERT_GT(test5, 0);
    ASSERT_NE(test5, test0);
    ASSERT_NE(test5, test1);
    ASSERT_NE(test5, test2);
    ASSERT_NE(test5, test3);

    // This is equal since _key can only test the first 4 chars
    ASSERT_EQ(test5, test4);
}
