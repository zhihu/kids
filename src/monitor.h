#ifndef __KIDS_MONITOR_H_
#define __KIDS_MONITOR_H_

#include <vector>
#include <string>
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
  static const uint32_t kCronPeriod   = 5;  // 5 second
  static const uint32_t kCronPeriodms = kCronPeriod * 1000;  // 5000 ms

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

  struct ClientAddress {
    std::string host;
    int port;
  };

  using TopicCount = std::unordered_map<sds, FlowCount, SdsHasher, SdsEqual>;
  using TopicSet   = std::unordered_set<sds, SdsHasher, SdsEqual>;
  using FdCount    = std::unordered_map<sds, FdSet, SdsHasher, SdsEqual>;

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
      sds topic_in_table;
      auto itr = topic_table.find(topic);
      if (itr == topic_table.end()) {
        topic_in_table = sdsdup(topic);
        topic_table.insert(topic_in_table);
      } else {
        topic_in_table = *itr;
      }
      return topic_in_table;
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
  std::vector<ClientAddress> GetInflowClients(const sds topic);
  std::vector<ClientAddress> GetOutflowClients(const sds topic);

  void UpdateTopicTable();

  void RegisterClient(int fd, const char *host, int port);
  void UnRegisterClient(int fd);

 private:
  Monitor();

 private:
  std::vector<Worker *> workers_;
  aeEventLoop *eventl_;
  long long cron_id_;

  Spinlock topic_lock;
  Spinlock host_lock;
  pthread_t monitor_thread_;

  std::unordered_map<int, ClientAddress> clients;

  FdCount    fds_by_topic_;
  TopicSet   active_topics_;
  TopicSet   topic_table_;
  TopicCount topic_count_;
};

#endif  // __KIDS_MONITOR_H_
