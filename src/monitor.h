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

typedef char *sds;
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

    void Merge(const TopicCount &count);
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

    void IncreaseTopicInflowCount(const sds topic, int fd);
    void IncreaseTopicOutflowCount(const sds topic, int fd);
    sds LocalizeTopic(const sds topic);
    ~Stats();
  };

  static Monitor *Create(const std::vector<Worker*> &workers_);
  ~Monitor();

  void Start();
  void Stop();
  void Cron();

  TopicSet GetActiveTopics();
  TopicSet GetAllTopics();
  TopicStats GetTopicStats();
  bool GetTopicCount(const sds topic, TopicCount *topic_count);
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
