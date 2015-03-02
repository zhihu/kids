#ifndef __KIDS_MASTER_H_
#define __KIDS_MASTER_H_

#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>

#include "conf.h"
#include "common.h"

typedef char* sds;

struct MessageQueue;
typedef struct MessageQueue MQ;

struct aeEventLoop;
class Storer;
class Worker;

class Master {
 public:
  typedef struct Config {
    int worker_threads;
    std::string listen_socket;
    std::string listen_host;
    int listen_port;
    bool ignore_case;
    int max_clients;
    LimitConfig nlimit[3];
    uint64_t queue_limit;
  } Config;

  static Master *Create(const KidsConfig *conf);
  ~Master();

  void AssignNewConnection(const int fd);
  bool PutMessage(const sds &topic, const sds &content, const int worker_id);

  void NotifyNewMessage(const int worker_id);

  bool ShouldAccept(const std::string &host);
  void RemoveClient(const std::string &host);

  void Start();
  void Stop();
  void ReloadStoreConfig(const KidsConfig* conf);
  void Cron();

  const Config config_;
  Statistic stat_;

 private:
  explicit Master(const KidsConfig *conf);

  int last_thread_;
  std::vector<Worker*> workers_;

  std::unordered_map<std::string, uint16_t> connected_clients_;
  std::mutex clients_mtx_;

  Storer *storer_;
  MQ *message_queue_;

  aeEventLoop *eventl_;
  int tcpfd_;
  int unixfd_;
  long long timeid_;
};

#endif  // __KIDS_MASTER_H_
