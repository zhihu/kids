#include <stdio.h>

#include <string>

#include "gtest/gtest.h"

#include "filesystem.h"

TEST(TestMakeName, Test1) {
  time_t now = time(NULL);
  struct tm tm;
  localtime_r(&now, &tm);
  std::string name = MakeName("abc", tm, "");
  EXPECT_EQ("abc", name);
}

TEST(TestMakeName, Test2) {
  char buffer[1024];
  time_t now = time(NULL);
  struct tm tm;
  localtime_r(&now, &tm);

  sprintf(buffer, "base-%04d-%02d-%02d-%02d-%02d-%02d-name.log",
          1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
          tm.tm_hour, tm.tm_min, tm.tm_sec);
  std::string name = MakeName("base-[date]-[time]-name.log", tm, "");
  EXPECT_EQ(buffer, name);
}

TEST(TestMakeName, Test3) {
  char buffer[1024];
  time_t now = time(NULL);
  struct tm tm;
  localtime_r(&now, &tm);

  sprintf(buffer, "base-[datea]-%02d-%02d-%02d-name.log",
          tm.tm_hour, tm.tm_min, tm.tm_sec);
  std::string name = MakeName("base-[datea]-[time]-name.log", tm, "");
  EXPECT_EQ(buffer, name);
}

TEST(TestMakeName, Test4) {
  time_t now = time(NULL);
  struct tm tm;
  localtime_r(&now, &tm);

  std::string name = MakeName("/tmp/kids/[topic]", tm, "nginx");
  EXPECT_EQ("/tmp/kids/nginx", name);
  name = MakeName("/tmp/kids/[topic]/logs/", tm, "nginx");
  EXPECT_EQ("/tmp/kids/nginx/logs/", name);
}

TEST(TestMakeName, Test5) {
  char buffer[1024];
  time_t now = time(NULL);
  struct tm tm;
  localtime_r(&now, &tm);

  sprintf(buffer, "/tmp/kidslog/tornado/%02d-%02d-%02d",
          tm.tm_hour, tm.tm_min, tm.tm_sec);
  std::string name = MakeName("/tmp/kidslog/[topic]/[time]", tm, "tornado");
  EXPECT_EQ(buffer, name);
}

TEST(TestFindRoot, Test1) {
  EXPECT_EQ(".", FindRoot("abc"));
  EXPECT_EQ("/", FindRoot("/abc"));
  EXPECT_EQ("/abc/", FindRoot("/abc/[topic]"));
  EXPECT_EQ("/abc/", FindRoot("/abc/[topic]/[date]/logs"));
  EXPECT_EQ("/tmp/", FindRoot("/tmp/abc"));
  EXPECT_EQ("/abc//kids//", FindRoot("/abc//kids//[topic]/[date]/logs"));
  EXPECT_EQ("/abc/", FindRoot("/abc/kids[topic]/[date]/logs"));
}
