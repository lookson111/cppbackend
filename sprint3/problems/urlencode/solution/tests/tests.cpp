#include <gtest/gtest.h>

#include "../src/urlencode.h"

using namespace std::literals;

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    EXPECT_EQ(UrlEncode(""sv), ""s);
    EXPECT_EQ(UrlEncode("hello"sv), "hello"s);
    EXPECT_EQ(UrlEncode("hello!"sv), "hello%21"s);
    EXPECT_EQ(UrlEncode("hello*"sv), "hello%2a"s);
    EXPECT_EQ(UrlEncode("моё почтение"sv), "%d0%bc%d0%be%d1%91%20%d0%bf%d0%be%d1%87%d1%82%d0%b5%d0%bd%d0%b8%d0%b5"s);
}   

/* Напишите остальные тесты самостоятельно */
