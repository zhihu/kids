#include "kids.h"
#include "store.h"
#include "filestore.h"
#include "networkstore.h"
#include "multiplestore.h"
#include "bufferstore.h"
#include "prioritystore.h"

Store *Store::Create(StoreConfig *conf, struct Statistic *stat, aeEventLoop *el) {
  Store *store = NULL;
  if (conf->type == "multiple") {
    store = new MultipleStore(conf, stat, el);
  } else if (conf->type == "buffer") {
    store = new BufferStore(conf, stat, el);
  } else if (conf->type == "network") {
    store = new NetworkStore(conf, el);
  } else if (conf->type == "file") {
    store = new FileStore(conf, stat);
  } else if (conf->type == "null") {
    store = new NullStore(conf);
  } else if (conf->type == "priority") {
    store = new PriorityStore(conf, stat, el);
  }
  return store;
}

Store::Store(StoreConfig *conf) : topic_(conf->topic) {
  if (conf->buffer_type == "primary") {
    buffer_type_ = kPrimary;
  } else if (conf->buffer_type == "secondary") {
    buffer_type_ = kSecondary;
  } else {
    buffer_type_ = kNone;
  }
}

bool Store::PreAddMessage(const Message *msg) {
  LogDebug("topic verify %s %s", topic_.c_str(), msg->topic);
  if (topic_.length() == 1 && topic_[0] == '*') return true;  // avoid string matching
  if (!stringmatchlen(topic_.c_str(), topic_.length(), msg->topic, sdslen(msg->topic), kids->config_.ignore_case)) {
    LogDebug("topic mismatch: %s - %s", topic_.c_str(), msg->topic);
    return false;
  }
  return true;
}

bool Store::AddMessage(const Message *msg) {
  if (PreAddMessage(msg)) {
    LogDebug("pre check ok");
    return DoAddMessage(msg);
  }
  return false;
}
