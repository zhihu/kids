#include <stdlib.h>
#include <string.h>

#include <fstream>

#include "conf.h"
#include "logger.h"
#include "parser.h"
#include "constants.h"
#include "util.h"

#define KEYWORD(ID) { \
  tk = new Token(ID, ts, te); \
  Parse(parser, ID, tk, ctx); \
}

#define SYMBOL(ID) { \
  Parse(parser, ID, NULL, ctx); \
}

#define VALUE() { \
  tk = new Token(TK_VALUE, ts + 1, te); \
  Parse(parser, TK_VALUE, tk, ctx); \
}

#define ID() { \
  tk = new Token(TK_ID, ts, te); \
  Parse(parser, TK_ID, tk, ctx); \
}

static void SetKidsConfValue(KidsConfig *conf, KeyValue *kv, ParseContext *ctx) {
  if (kv->key == "level") {
    conf->log_level = kv->value[0];
  } else if (kv->key == "file") {
    conf->log_file = kv->value[0];
  } else if (kv->key == "socket") {
    conf->listen_socket = kv->value[0];
  } else if (kv->key == "host") {
    conf->listen_host = kv->value[0];
  } else if (kv->key == "port") {
    conf->listen_port = kv->value[0];
  } else if (kv->key == "max_clients") {
    conf->max_clients = kv->value[0];
  } else if (kv->key == "worker_threads") {
    conf->worker_threads = kv->value[0];
  } else if (kv->key == "ignore_case") {
    conf->ignore_case = kv->value[0];
  } else if (kv->key == "nlimit") {
    if (kv->value.size() != 4) goto ERROR;
    int type = 0;
    if (kv->value[0] == "normal") {
      type = NLIMIT_NORMAL;
    } else if (kv->value[0] == "pubsub") {
      type = NLIMIT_PUBSUB;
    } else if (kv->value[0] == "networkstore") {
      type = NLIMIT_NETWORKSTORE;
    } else {
      goto ERROR;
    }
    if (!ParseSize(kv->value[1].c_str(), &conf->nlimit[type].hard_limit_bytes)) goto ERROR;
    if (!ParseSize(kv->value[2].c_str(), &conf->nlimit[type].soft_limit_bytes)) goto ERROR;
    if (!ParseTime(kv->value[3].c_str(), &conf->nlimit[type].soft_limit_seconds)) goto ERROR;
  }
  return;

ERROR:
  ctx->success = false;
  snprintf(ctx->error, 1024, "%s(%d): %s %s",
           ERR_CONF_UNKOWN_KIDS_FIELD, ctx->line, kv->key.c_str(), kv->value[0].c_str());
}

static void SetStoreConfValue(StoreConfig *conf, KeyValue *kv, ParseContext *ctx) {
  if (kv->key == "host") {
    conf->host = kv->value[0];
  } else if (kv->key == "port") {
    conf->port = kv->value[0];
  } else if (kv->key == "socket") {
    conf->socket = kv->value[0];
  } else if (kv->key == "path") {
    conf->path = kv->value[0];
  } else if (kv->key == "name") {
    conf->name = kv->value[0];
  } else if (kv->key == "rotate") {
    conf->rotate = kv->value[0];
  } else if (kv->key == "flush") {
    conf->flush = kv->value[0];
  } else if (kv->key == "success") {
    conf->success = kv->value[0];
  } else if (kv->key == "topic") {
    conf->topic = kv->value[0];
  } else {
    ctx->success = false;
    snprintf(ctx->error, 1024, "%s(%d): %s %s",
             ERR_CONF_UNKOWN_STORE_FIELD, ctx->line, kv->key.c_str(), kv->value[0].c_str());
  }
}

bool CheckStoreConfig(StoreConfig *conf, ParseContext *ctx) {
  if (conf->type == "multiple" || conf->type == "priority") {
    if (conf->stores.size() == 0) {
      snprintf(ctx->error, 1024, ERR_CONF_AT_LEAST_1);
      return false;
    }
    for (int i = 0; i < conf->stores.size(); i++) {
      if (!CheckStoreConfig(conf->stores[i], ctx)) return false;
    }
  } else if (conf->type == "buffer") {
    if (conf->stores.size() != 2) {
      snprintf(ctx->error, 1024, ERR_CONF_BUF_1);
      return false;
    }
    if (conf->stores[1]->buffer_type == "primary" && conf->stores[0]->buffer_type == "secondary") {
      std::swap(conf->stores[0], conf->stores[1]);
    } else if (!(conf->stores[0]->buffer_type == "primary" && conf->stores[1]->buffer_type == "secondary")) {
      snprintf(ctx->error, 1024, ERR_CONF_BUF_2);
      return false;
    }
    if (!CheckStoreConfig(conf->stores[0], ctx) || !CheckStoreConfig(conf->stores[1], ctx)) return false;
  } else if (conf->type == "network") {
    if (conf->socket.empty() && (conf->host.empty() || conf->port.empty())) {
      snprintf(ctx->error, 1024, ERR_BAD_ADDR);
      return false;
    }
  } else if (conf->type == "file") {
  } else if (conf->type == "null") {
  } else {
    snprintf(ctx->error, 1024, ERR_CONF_UNKOWN_STORE_TYPE);
    return false;
  }

  if (conf->topic.empty()) {
    conf->topic = "*";
  }

  return true;
}

void CheckKidsConfig(KidsConfig *conf, ParseContext *ctx) {
  if (conf->store == NULL) {
    conf->store = new StoreConfig();
    conf->store->type = "null";
    conf->store->topic = "*";
  } else {
    ctx->success = CheckStoreConfig(conf->store, ctx);
  }
}

#include "parser.c"
#include "lexer.c"

ParseContext *ParseConfigFile(const std::string& filename) {
  std::string str;
  std::string line;
  std::ifstream fin;
  fin.open(filename.c_str());
  while (std::getline(fin, line)) {
    str += line;
    str += "\n";
  }
  fin.close();
  return ParseConfig(str);
}

