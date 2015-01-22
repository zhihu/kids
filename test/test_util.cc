#include "gtest/gtest.h"

#include "util.h"

TEST(TestParseTime, Test1) {
  int t;
  EXPECT_TRUE(ParseTime("2h", &t));
  EXPECT_EQ(3600 * 2, t);
  EXPECT_TRUE(ParseTime("2hour", &t));
  EXPECT_EQ(3600 * 2, t);
  EXPECT_TRUE(ParseTime("3m", &t));
  EXPECT_EQ(60 * 3, t);
  EXPECT_TRUE(ParseTime("3min", &t));
  EXPECT_EQ(60 * 3, t);
  EXPECT_TRUE(ParseTime("40s", &t));
  EXPECT_EQ(40, t);
  EXPECT_TRUE(ParseTime("40sec", &t));
  EXPECT_EQ(40, t);
  EXPECT_TRUE(ParseTime("3d", &t));
  EXPECT_EQ(3 * 3600 * 24, t);
  EXPECT_TRUE(ParseTime("3day", &t));
  EXPECT_EQ(3 * 3600 * 24, t);

  EXPECT_FALSE(ParseTime("1f", &t));
  EXPECT_FALSE(ParseTime("40", &t));
  EXPECT_FALSE(ParseTime("-3d", &t));
}

TEST(TestParseSize, Test1) {
  int t;
  EXPECT_TRUE(ParseSize("100b", &t));
  EXPECT_EQ(100, t);
  EXPECT_TRUE(ParseSize("2k", &t));
  EXPECT_EQ(2 * 1024, t);
  EXPECT_TRUE(ParseSize("3kb", &t));
  EXPECT_EQ(3 * 1024, t);
  EXPECT_TRUE(ParseSize("3m", &t));
  EXPECT_EQ(3 * 1024 * 1024, t);
  EXPECT_TRUE(ParseSize("3mb", &t));
  EXPECT_EQ(3 * 1024 * 1024, t);
  EXPECT_TRUE(ParseSize("3G", &t));
  EXPECT_EQ(3 * 1024 * 1024 * 1024, t);

  EXPECT_FALSE(ParseSize("1f", &t));
  EXPECT_FALSE(ParseSize("40", &t));
  EXPECT_FALSE(ParseSize("-3d", &t));
}

TEST(TestBytesToHuman, Test1) {
  uint64_t n;
  char str[64];
  n = 42;
  bytesToHuman(str, n);
  EXPECT_STREQ("42B", str);

  memset(str, 0, sizeof(str));
  n = 42 * 1024 + 42;
  bytesToHuman(str, n);
  EXPECT_STREQ("42.04K", str);

  memset(str, 0, sizeof(str));
  n = 42LL * 1024 * 1024 + 42 * 1024 + 42;
  bytesToHuman(str, n);
  EXPECT_STREQ("42.04M", str);

  memset(str, 0, sizeof(str));
  n = 42LL * 1024 * 1024 * 1024 + 42 * 1024 * 1024 + 42 * 1024 + 42;
  bytesToHuman(str, n);
  EXPECT_STREQ("42.04G", str);

  memset(str, 0, sizeof(str));
  n = 42LL * 1024 * 1024 * 1024 * 1024 +
      42LL * 1024 * 1024 * 1024 +
      42 * 1024 * 1024 +
      42 * 1024 +
      42;
  bytesToHuman(str, n);
  EXPECT_STREQ("42.04T", str);
}
