#include "gtest/gtest.h"

#include "kids.h"
#include "util.h"

TEST(TestParseMessage, Test1) {
  char str[] = "nginx|[Tue, 22 Nov 2011 08:43:41] r6z4exqgaft5ruhrd6jy8gvahpamg6xjhr5hpojdfysa7ijgh4chn8qezti2k4k6ckzzlpf07x8b6fxpfnr07gl1ls431qwgc6";
  Message *msg = ParseMessage(str, sizeof(str) - 1);
  EXPECT_STREQ("nginx", msg->topic);
  EXPECT_STREQ("[Tue, 22 Nov 2011 08:43:41] r6z4exqgaft5ruhrd6jy8gvahpamg6xjhr5hpojdfysa7ijgh4chn8qezti2k4k6ckzzlpf07x8b6fxpfnr07gl1ls431qwgc6",
               msg->content);
}

TEST(TestParseMessage, Test2) {
  char str[] = "[Tue, 22 Nov 2011 08:43:41] r6z4exqgaft5ruhrd6jy8gvahpamg6xjhr5hpojdfysa7ijgh4chn8qezti2k4k6ckzzlpf07x8b6fxpfnr07gl1ls431qwgc6";
  Message *msg = ParseMessage(str, sizeof(str) - 1);
  EXPECT_STREQ("zhihu.notopic", msg->topic);
  EXPECT_STREQ("[Tue, 22 Nov 2011 08:43:41] r6z4exqgaft5ruhrd6jy8gvahpamg6xjhr5hpojdfysa7ijgh4chn8qezti2k4k6ckzzlpf07x8b6fxpfnr07gl1ls431qwgc6",
               msg->content);
}

TEST(TestParseMessage, Test3) {
  char str[] = "tornado|";
  Message *msg = ParseMessage(str, sizeof(str) - 1);
  EXPECT_STREQ("tornado", msg->topic);
  EXPECT_STREQ("", msg->content);
}

TEST(TestParseMessage, Test4) {
  char str[] = "tornado|nginx|log details";
  Message *msg = ParseMessage(str, sizeof(str) - 1);
  EXPECT_STREQ("tornado", msg->topic);
  EXPECT_STREQ("nginx|log details", msg->content);
}
