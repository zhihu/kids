#ifndef __KIDS_MONITOR_H_
#define __KIDS_MONITOR_H_

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "common.h"
#include "spinlock.h"

class Worker;
class Client;

typedef char * sds;
struct aeEventLoop;

class Monitor {
 public:
  static const uint32_t kCronPeriod   = 5;  // 5 second
  static const uint32_t kCronPeriodms = kCronPeriod * 1000;  // 5000 ms

  struct TopicCount {
    uint32_t inflow_count;
    uint32_t outflow_count;
    std::unordered_set<int> inflow_clients;
    std::unordered_set<int> outflow_clients;

    void Merge(const TopicCount &count) {
      inflow_count += count.inflow_count;
      outflow_count += count.outflow_count;
      inflow_clients.insert(count.inflow_clients.begin(), count.inflow_clients.end());
      outflow_clients.insert(count.outflow_clients.begin(), count.outflow_clients.end());
    }
  };

  struct ClientAddress {
    std::string host;
    int port;
  };

  using TopicStats = std::unordered_map<sds, TopicCount, SdsHasher, SdsEqual>;
  using TopicSet   = std::unordered_set<sds, SdsHasher, SdsEqual>;

  struct Stats {
    TopicStats topic_stats;
    TopicSet   topic_table;
    Spinlock   splock;

    void IncreaseTopicInflowCount(const sds topic, int fd) {
      sds local_topic = LocalizeTopic(topic);
      splock.Lock();
      topic_stats[local_topic].inflow_count += 1;
      topic_stats[local_topic].inflow_clients.insert(fd);
      splock.Unlock();
    }

    void IncreaseTopicOutflowCount(const sds topic, int fd) {
      sds local_topic = LocalizeTopic(topic);
      splock.Lock();
      topic_stats[local_topic].outflow_count += 1;
      topic_stats[local_topic].outflow_clients.insert(fd);
      splock.Unlock();
    }

    ~Stats() {
      for (auto &topic : topic_table) {
        sdsfree(topic);
      }
    }

   private:
    sds LocalizeTopic(const sds topic) {
      sds local_topic;
      auto itr = topic_table.find(topic);
      if (itr == topic_table.end()) {
        local_topic = sdsdup(topic);
        topic_table.insert(local_topic);
      } else {
        local_topic = *itr;
      }
      return local_topic;
    }
  };

  static Monitor *Create(const std::vector<Worker*> &workers_);
  ~Monitor();

  void Start();
  void Stop();
  void Cron();

  TopicSet GetActiveTopics();
  TopicSet GetAllTopics();
  TopicStats GetTopicStats();
  bool GetTopicCount(const sds topic, TopicCount &topic_count);
  std::vector<ClientAddress> GetPublisher(const sds topic);
  std::vector<ClientAddress> GetSubscriber(const sds topic);

  void CollectStats();

  void RegisterClient(int fd, const char *host, int port);
  void UnRegisterClient(int fd);

 private:
  Monitor();

 private:
  std::vector<Worker *> workers_;
  aeEventLoop *eventl_;
  long long cron_id_;

  Spinlock topic_lock_;
  Spinlock host_lock_;
  pthread_t monitor_thread_;

  std::unordered_map<int, ClientAddress> clients_;

  TopicStats topic_stats_;
  TopicSet   topic_table_;
};

#endif  // __KIDS_MONITOR_H_
