#include "gtest/gtest.h"

#include "util.h"

TEST(TestPattern, TestFullMatch) {
  EXPECT_EQ(1, stringmatch("www_zhihu_com_error", "www_zhihu_com_error", 0));
}

TEST(TestPattern, TestStar) {
  EXPECT_EQ(1, stringmatch("www_*", "www_zhihu_com_error", 0));
  EXPECT_EQ(1, stringmatch("zhihu.*", "zhihu.antispam.log", 0));
  EXPECT_EQ(1, stringmatch("zhihu.*", "zhihu.request", 0));
  EXPECT_EQ(1, stringmatch("*.request", "zhihu.request", 0));
  EXPECT_EQ(0, stringmatch("*.request", "zhihu.response", 0));
  EXPECT_EQ(1, stringmatch("zhihu.*.access", "zhihu.nginx.access", 0));
  EXPECT_EQ(1, stringmatch("zhihu.*.access", "zhihu.tornado.noti.access", 0));
  EXPECT_EQ(0, stringmatch("zhihu.*.access", "zhihu.tornado.error", 0));
}

TEST(TestPattern, TestCase) {
  EXPECT_EQ(0, stringmatch("www_*", "WWW_ZHIHU_COM_ERROR", 0));
  EXPECT_EQ(1, stringmatch("ZHIHU.*", "ZHIHU.antispam.log", 0));
  EXPECT_EQ(1, stringmatch("zhihu.*", "ZHIHU.REQUEST", 1));
}
