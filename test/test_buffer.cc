#include <vector>

#include "gtest/gtest.h"

#include "buffer.h"
#include "util.h"

TEST(TestBuffer, TestConstruct) {
  Buffer buf;
  EXPECT_EQ(0, buf.size());
  EXPECT_EQ(0, buf.capacity());
  EXPECT_EQ(0, buf.blank_size());

  Buffer *strs = new Buffer("abc def ghi");
  EXPECT_TRUE(strs != NULL);
  EXPECT_EQ(11, strs->size());
  EXPECT_STREQ("abc def ghi", strs->data());
  delete strs;
}

TEST(TestBuffer, TestAppend) {
  Buffer buf;
  buf.append("abc", 4);
  EXPECT_EQ(4, buf.size());
  EXPECT_STREQ("abc", buf.data());
  EXPECT_EQ('b', buf[1]);

  buf.append("def", 4);
  EXPECT_EQ(8, buf.size());
  EXPECT_EQ('a', buf[0]);
  EXPECT_EQ('b', buf[1]);
  EXPECT_EQ('c', buf[2]);
  EXPECT_EQ('\0', buf[3]);
  EXPECT_EQ('d', buf[4]);
  EXPECT_EQ('e', buf[5]);
  EXPECT_EQ('f', buf[6]);
  EXPECT_EQ('\0', buf[7]);
}

TEST(TestBuffer, TestResize) {
  Buffer buf;
  buf.resize(10);
  EXPECT_EQ(10, buf.capacity());
  EXPECT_EQ('\0', buf.data()[10]);
}

TEST(TestBuffer, TestCopy) {
  Buffer buf;
  buf.resize(10);
  EXPECT_EQ(10, buf.capacity());

  Buffer buf2(buf);
  EXPECT_EQ(0, buf2.size());
  EXPECT_EQ(10, buf2.capacity());
  buf2.append("abc", 4);
  EXPECT_STREQ("abc", buf2.data());
  EXPECT_EQ(10, buf2.capacity());

  Buffer buf3(buf2);
  EXPECT_EQ(4, buf3.size());
  EXPECT_EQ(10, buf3.capacity());
  EXPECT_STREQ("abc", buf3.data());
}

TEST(TestBuffer, TestPopFront) {
  Buffer strs("abc def ghi");
  EXPECT_STREQ("abc def ghi", strs.data());
  Buffer *buf = strs.pop_front(4);
  EXPECT_TRUE(buf != NULL);
  EXPECT_STREQ("abc ", buf->data());
  EXPECT_EQ(7, strs.size());
  EXPECT_STREQ("def ghi", strs.data());
  delete buf;

  buf = strs.pop_front(strs.size());
  EXPECT_EQ(0, strs.size());
  EXPECT_STREQ("def ghi", buf->data());
}

TEST(TestBuffer, TestRemoveFront) {
  Buffer strs("abc def ghi");
  Buffer buf = strs.data();
  EXPECT_STREQ("abc def ghi", buf.data());
  buf.remove_front(4);
  EXPECT_STREQ("def ghi", buf.data());
  buf.remove_front(4);
  EXPECT_STREQ("ghi", buf.data());
  buf.remove_front(3);
  EXPECT_STREQ("", buf.data());
}

TEST(TestBuffer, TestAssign) {
  Buffer strs("abc def ghi");
  strs = "jkl";
  EXPECT_STREQ("jkl", strs.data());
  std::vector<Buffer> v;
  v.push_back(Buffer());
  EXPECT_STREQ("", v.front().data());
}

TEST(TestBuffer, TestClear) {
  Buffer strs("abc def ghi");
  Buffer strs2 = strs;
  EXPECT_EQ(2, strs.refcount());
  strs2.clear();
  EXPECT_EQ(11, strs.size());
  EXPECT_EQ(1, strs.refcount());
  EXPECT_EQ(1, strs2.refcount());
}


TEST(TestBuffer, TestHashset) {
  std::unordered_set<Buffer, Buffer::Hasher> set;
  Buffer buf("abc def ghi");
  set.insert(buf);
  EXPECT_EQ(1, set.size());
  EXPECT_EQ(2, buf.refcount());
  Buffer buf2("abc def ghi");
  EXPECT_EQ(1, buf2.refcount());
  std::unordered_set<Buffer, Buffer::Hasher>::iterator it = set.find(buf2);
  EXPECT_EQ(2, it->refcount());
  set.erase(it);
  EXPECT_EQ(1, buf.refcount());
}

TEST(TestBuffer, TestHashmap) {
  std::unordered_map<Buffer, int, Buffer::Hasher> int_by_topic;
  std::unordered_set<Buffer, Buffer::Hasher> sub_topics;
  Buffer *buf = new Buffer("abc");
  EXPECT_EQ(1, buf->refcount());
  Buffer& topic = *buf;
  sub_topics.insert(topic);
  EXPECT_EQ(2, topic.refcount());
  int_by_topic[topic] = 1;
  EXPECT_EQ(3, topic.refcount());
  delete buf;

  Buffer buf2("abc");
  std::unordered_set<Buffer, Buffer::Hasher>::iterator it = sub_topics.find(buf2);
  EXPECT_TRUE(it != sub_topics.end());
  EXPECT_EQ(2, it->refcount());
  //sub_topics.erase(it);
}
