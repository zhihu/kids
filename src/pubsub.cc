#include "kids.h"

void Client::ProcessSubscribe() {
  if (argv_.size() < 2) {
    ReplyErrorFormat("wrong number of arguments for 'subscribe' command");
    return;
  }

  // Add the channel to the client -> channels hash table
  for (int i = 1; i < argv_.size(); i++) {
    sds topic = argv_[i];

    if (sub_topics_.find(topic) == sub_topics_.end()) {
      // transfer ownership to sub_topics_
      sub_topics_.insert(argv_[i]);
      argv_[i] = NULL;

      // Add the client to the channel -> list of clients hash table
      auto it = worker_->clients_by_topic_.find(topic);
      if (it == worker_->clients_by_topic_.end()) {
        worker_->clients_by_topic_.emplace(sdsdup(topic), std::list<Client*>(1, this));
      } else {
        it->second.push_back(this);
      }
    }

    // Notify the client
    Reply(REP_MBULK3, REP_MBULK3_SIZE);
    Reply(REP_SUBSCRIBEBULK, REP_SUBSCRIBEBULK_SIZE);
    ReplyBulk(topic, sdslen(topic));
    ReplyLongLong(sub_topics_.size() + sub_patterns_.size());
  }
}

void Client::UnsubscribeAllTopics(bool notify) {
  while (sub_topics_.size() > 0) {
    UnsubscribeTopic(*sub_topics_.begin(), notify);
  }
}

void Client::UnsubscribeAllPatterns(bool notify) {
  while (sub_patterns_.size() > 0) {
    UnsubscribePattern(*sub_patterns_.begin(), notify);
  }
}

void Client::ProcessPSubscribe() {
  if (argv_.size() < 2) {
    ReplyErrorFormat("wrong number of arguments for 'psubscribe' command");
    return;
  }

  for (int i = 1; i < argv_.size(); i++) {
    sds pattern = argv_[i];
    auto it = sub_patterns_.find(pattern);
    if (it == sub_patterns_.end()) {
      sub_patterns_.insert(pattern);
      worker_->pubsub_patterns_.push_back(new Pattern(pattern, this));
      argv_[i] = NULL;
    }

    // Notify the client
    Reply(REP_MBULK3, REP_MBULK3_SIZE);
    Reply(REP_PSUBSCRIBEBULK, REP_PSUBSCRIBEBULK_SIZE);
    ReplyBulk(pattern, sdslen(pattern));
    ReplyLongLong(sub_topics_.size() + sub_patterns_.size());
  }
}

void Client::ProcessUnSubscribe(bool notify) {
  if (argv_.size() > 1) {
    for (int i = 1; i < argv_.size(); i++) {
      UnsubscribeTopic(argv_[i], notify);
    }
  } else {
    UnsubscribeAllTopics(notify);
  }
}

void Client::ProcessPUnSubscribe(bool notify) {
  if (argv_.size() > 1) {
    for (int i = 1; i < argv_.size(); i++) {
      UnsubscribePattern(argv_[i], notify);
    }
  } else {
    UnsubscribeAllPatterns(notify);
  }
}

void Client::UnsubscribeTopic(const sds& topic, bool notify) {
  LogDebug("Unsubscribe %s", topic);
  // Notify the client
  if (notify) {
    Reply(REP_MBULK3, REP_MBULK3_SIZE);
    Reply(REP_UNSUBSCRIBEBULK, REP_UNSUBSCRIBEBULK_SIZE);
    ReplyBulk(topic, sdslen(topic));
    ReplyLongLong(sub_topics_.size() + sub_patterns_.size());
  }

  auto it = sub_topics_.find(topic);
  if (it != sub_topics_.end()) {
    // Remove the client from the channel -> clients list hash table
    auto clients = worker_->clients_by_topic_.find(topic);
    // when called from UnsubscribeAllTopics, topic == (*it)
    // so do not use topic after free (*it)
    sdsfree(*it);
    sub_topics_.erase(it);
    clients->second.remove(this);

    if (clients->second.size() == 0) {
      // Free the list and associated hash entry at all if this was
      // the latest client, so that it will be possible to abuse
      // Redis PUBSUB creating millions of channels.
      sdsfree(clients->first);
      worker_->clients_by_topic_.erase(clients);
    }
  }
}

void Client::UnsubscribePattern(const sds& pattern, bool notify) {
  // Notify the client
  if (notify) {
    Reply(REP_MBULK3, REP_MBULK3_SIZE);
    Reply(REP_PUNSUBSCRIBEBULK, REP_PUNSUBSCRIBEBULK_SIZE);
    ReplyBulk(pattern, sdslen(pattern));
    ReplyLongLong(sub_topics_.size() + sub_patterns_.size());
  }

  auto it = sub_patterns_.find(pattern);
  if (it != sub_patterns_.end()) {
    for (auto p = worker_->pubsub_patterns_.begin();
         p != worker_->pubsub_patterns_.end(); ++p) {
      if (SdsEqual()((*p)->pattern, pattern) && (*p)->client == this) {
        delete (*p);
        worker_->pubsub_patterns_.erase(p);
        break;
      }
    }
    // pattern string is shared by sub_patterns_ and pubsub_patterns_
    // when called by UnsubscribeAllTopics, (*it) = pattern
    // so do not use pattern after free (*it)
    sdsfree(*it);
    sub_patterns_.erase(it);
  }
}
