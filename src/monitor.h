#ifndef __KIDS_MONITOR_H_
#define __KIDS_MONITOR_H_

#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "./common.h"
#include "./spinlock.h"

class Worker;
class Client;

typedef char * sds;
struct aeEventLoop;

class Monitor {
 public:
  static const uint32_t kCronPeriod   = 10;  // 10 second
  static const uint32_t kCronPeriodms = kCronPeriod * 1000;  // 1e4 ms

  struct FlowCount {
    uint32_t in;
    uint32_t out;
    void Add(const FlowCount &count) {
      in += count.in;
      out += count.out;
    }
  };

  using TopicCount = std::unordered_map<sds, FlowCount, SdsHasher, SdsEqual>;
  using TopicSet = std::unordered_set<sds, SdsHasher, SdsEqual>;

  struct Stats {
    TopicCount topic_count;
    TopicSet topic_table;
    Spinlock splock;

    void IncreaseTopicInflowCount(const sds topic) {
      sds dup_topic;
      auto itr = topic_table.find(topic);
      if (itr == topic_table.end()) {
        dup_topic = sdsdup(topic);
        topic_table.insert(dup_topic);
      } else {
        dup_topic = *itr;
      }
      splock.Lock();
      topic_count[dup_topic].in += 1;
      splock.Unlock();
    }

    void IncreaseTopicOutflowCount(const sds topic) {
      sds dup_topic;
      auto itr = topic_table.find(topic);
      if (itr == topic_table.end()) {
        dup_topic = sdsdup(topic);
        topic_table.insert(dup_topic);
      } else {
        dup_topic = *itr;
      }
      splock.Lock();
      topic_count[dup_topic].out += 1;
      splock.Unlock();
    }

    ~Stats(){
      for (auto &topic : topic_table) {
        sdsfree(topic);
      }
    }
  };

  static Monitor *Create(const std::vector<Worker*> &workers_);
  ~Monitor();

  void Start();
  void Stop();
  void Cron();

  TopicSet GetActiveTopics();
  TopicSet GetAllTopics();
  TopicCount GetTopicCount();

  void UpdateTopicTable();

 private:
  Monitor();

 private:
  std::vector<Worker *> workers_;
  aeEventLoop *eventl_;
  long long cron_id_;

  Spinlock topic_lock;
  pthread_t monitor_thread_;

  TopicSet active_topics_;
  TopicSet topic_table_;
  TopicCount topic_count_;
};

#endif  // __KIDS_MONITOR_H_
