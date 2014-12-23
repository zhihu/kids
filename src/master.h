#ifndef __KIDS_MASTER_H_
#define __KIDS_MASTER_H_

#include <vector>
#include <string>

#include "conf.h"
#include "common.h"

typedef char* sds;

struct MessageQueue;
typedef struct MessageQueue MQ;

struct aeEventLoop;
class Monitor;
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
    bool monitor;
    LimitConfig nlimit[3];
    uint64_t queue_limit;
  } Config;

  static Master *Create(const KidsConfig *conf);
  ~Master();

  void AssignNewConnection(const int fd);
  bool PutMessage(const sds &topic, const sds &content, const int worker_id);

  void NotifyNewMessage(const int worker_id);

  void Start();
  void Stop();
  void Cron();

  const Config config_;
  Statistic stat_;
  Monitor *monitor_;

 private:
  explicit Master(const KidsConfig *conf);

  int last_thread_;
  std::vector<Worker*> workers_;

  Storer *storer_;
  MQ *message_queue_;

  aeEventLoop *eventl_;
  int tcpfd_;
  int unixfd_;
  long long timeid_;
};

#endif  // __KIDS_MASTER_H_
