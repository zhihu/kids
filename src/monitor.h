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

  struct FdSet {
    std::unordered_set<int> inflow;
    std::unordered_set<int> outflow;
  };

  using TopicCount = std::unordered_map<sds, FlowCount, SdsHasher, SdsEqual>;
  using TopicSet = std::unordered_set<sds, SdsHasher, SdsEqual>;
  using FdCount = std::unordered_map<sds, FdSet, SdsHasher, SdsEqual>;

  struct Stats {
    TopicCount topic_count;
    TopicSet topic_table;
    Spinlock splock;
    FdCount fds_by_topic;

    void IncreaseTopicInflowCount(const sds topic, int fd) {
      sds topic_in_table = UpdateTable(topic);
      splock.Lock();
      topic_count[topic_in_table].in += 1;
      fds_by_topic[topic_in_table].inflow.insert(fd);
      splock.Unlock();
    }

    void IncreaseTopicOutflowCount(const sds topic, int fd) {
      sds topic_in_table = UpdateTable(topic);
      splock.Lock();
      topic_count[topic_in_table].out += 1;
      fds_by_topic[topic_in_table].outflow.insert(fd);
      splock.Unlock();
    }

    ~Stats() {
      for (auto &topic : topic_table) {
        sdsfree(topic);
      }
    }

   private:
    sds UpdateTable(const sds topic) {
      sds dup_topic;
      auto itr = topic_table.find(topic);
      if (itr == topic_table.end()) {
        dup_topic = sdsdup(topic);
        topic_table.insert(dup_topic);
      } else {
        dup_topic = *itr;
      }
      return dup_topic;
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

  FdCount fds_by_topic_;
  TopicSet active_topics_;
  TopicSet topic_table_;
  TopicCount topic_count_;
};

#endif  // __KIDS_MONITOR_H_
