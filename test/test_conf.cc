#include "gtest/gtest.h"

#include "conf.h"
#include "logger.h"
#include "constants.h"

TEST(TestParseConfig, TestConf) {
  char str[] = "listen {\n socket /tmp/kids.sock;\n}";
  ParseContext *ctx = ParseConfig(str);
  EXPECT_TRUE(ctx->success);

  KidsConfig *conf = ctx->conf;
  EXPECT_FALSE(conf == NULL);
}

TEST(TestParseConfig, TestSyntaxError1) {
  char str[] = "listen \n socket /tmp/kids.sock;\n}";
  ParseContext *ctx = ParseConfig(str);
  EXPECT_FALSE(ctx->success);
  EXPECT_TRUE(ctx->conf == NULL);
}

TEST(TestParseConfig, TestStr3) {
  char str[] = "listen {\n socket /tmp/kids.sock;\n} store file { path /tmp/; name abc; }";
  ParseContext *ctx = ParseConfig(str);
  EXPECT_TRUE(ctx->success);

  KidsConfig *conf = ctx->conf;
  EXPECT_FALSE(conf == NULL);
  EXPECT_EQ("/tmp/", conf->store->path);
  EXPECT_EQ("abc", conf->store->name);
}

TEST(TestParseConfig, TestFile1) {
  ParseContext *ctx = ParseConfigFile("../abc.conf");
  EXPECT_FALSE(ctx->success);
  EXPECT_STREQ("Bad or empty config file", ctx->error);
}

TEST(TestParseConfig, TestFileAgentConf) {
  ParseContext *ctx = ParseConfigFile(TEST_DIR"/conf/agent.conf");
  EXPECT_TRUE(ctx->success);
  EXPECT_FALSE(ctx->conf == NULL);

  KidsConfig *conf = ctx->conf;
  EXPECT_EQ("debug", conf->log_level);
  EXPECT_EQ("/tmp/kids.log", conf->log_file);
  EXPECT_EQ("/tmp/kids.sock", conf->listen_socket);
  EXPECT_EQ("", conf->listen_host);
  EXPECT_EQ("", conf->listen_port);
  EXPECT_EQ("on", conf->ignore_case);

  StoreConfig *store = conf->store;
  EXPECT_EQ("multiple", store->type);
  EXPECT_EQ(2, store->stores.size());

  StoreConfig *store1 = store->stores[0];
  EXPECT_EQ("buffer", store1->type);
  EXPECT_EQ("primary", store1->stores[0]->buffer_type);
  EXPECT_EQ("network", store1->stores[0]->type);
  EXPECT_EQ("secondary", store1->stores[1]->buffer_type);
  EXPECT_EQ("file", store1->stores[1]->type);
}

TEST(TestParseConfig, TestFileServerConf) {
  ParseContext *ctx = ParseConfigFile(TEST_DIR"/conf/server.conf");
  EXPECT_TRUE(ctx->success);
  EXPECT_FALSE(ctx->conf == NULL);

  KidsConfig *conf = ctx->conf;
  EXPECT_EQ("error", conf->log_level);
  EXPECT_EQ("/tmp/kids_err.log", conf->log_file);
  EXPECT_EQ("", conf->listen_socket);
  EXPECT_EQ("127.0.0.1", conf->listen_host);
  EXPECT_EQ("3434", conf->listen_port);
  EXPECT_EQ("200", conf->max_clients);
  EXPECT_EQ("off", conf->ignore_case);

  StoreConfig *store = conf->store;
  EXPECT_EQ("multiple", store->type);
  EXPECT_EQ(2, store->stores.size());

  StoreConfig *store1 = store->stores[0];
  EXPECT_EQ("file", store1->type);
  EXPECT_EQ("/tmp/kids/", store1->path);
  EXPECT_EQ("abc", store1->name);

  StoreConfig *store2 = store->stores[1];
  EXPECT_EQ("network", store2->type);
  EXPECT_EQ("127.0.0.1", store2->host);
  EXPECT_EQ("3435", store2->port);
}
