#ifndef __KIDS_CONFIG_H_
#define __KIDS_CONFIG_H_

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#define NLIMIT_NORMAL 0
#define NLIMIT_NETWORKSTORE 1
#define NLIMIT_PUBSUB 2

struct LimitConfig {
  int hard_limit_bytes;
  int soft_limit_bytes;
  int soft_limit_seconds;
};

struct StoreConfig {
  ~StoreConfig() {
    for (std::vector<StoreConfig*>::iterator it = stores.begin(); it != stores.end(); ++it) {
      delete *it;
    }
  }

  std::string type;
  std::string buffer_type;
  std::string socket;
  std::string host;
  std::string port;
  std::string path;
  std::string name;
  std::string rotate;
  std::string flush;
  std::string success;
  std::string topic;
  std::vector<StoreConfig*> stores;
};

struct KidsConfig {
  KidsConfig() : store(NULL) { memset(nlimit, 0, sizeof(nlimit)); }
  ~KidsConfig() { delete store; }
  std::string listen_socket;
  std::string listen_host;
  std::string listen_port;
  std::string log_level;
  std::string log_file;
  std::string max_clients;
  std::string worker_threads;
  std::string ignore_case;
  LimitConfig nlimit[3];
  StoreConfig *store;
};

struct Token {
  Token() {}
  Token(int tid, char *s, char *e) {
    id = tid;
    while (s < e) {
      value.push_back(*s);
      s++;
    }
  }
  int id;
  std::string value;
};

struct KeyValue {
  KeyValue(std::string k, std::vector<std::string>* v)
      :key(k), value(*v) {}
  std::string key;
  std::vector<std::string> value;
};

struct ParseContext {
  ParseContext() : line(0), success(true), conf(NULL) {}
  ~ParseContext() { delete conf; }

  int line;
  bool success;
  char error[1025];
  KidsConfig *conf;
};

ParseContext *ParseConfigFile(const std::string& filename);
ParseContext *ParseConfig(std::string str);

#endif  // __KIDS_CONFIG_H_
