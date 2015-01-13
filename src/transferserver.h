#ifndef __KIDS_TRANSFERSERVER_H
#define __KIDS_TRANSFERSERVER_H

#include <mutex>
#include <unordered_map>
#include <deque>

#include "filesystem.h"
#include "common.h"

typedef char *sds;
struct aeEventLoop;
struct StoreConfig;

struct BufferedMessage {
  sds timestamp;
  sds topic;
  sds content;
};

class TransferServer {
 public:
  static const int kMaxMessagePerCron = 100;
  static const int kInitCronPeriod    = 5000;
  using TopicFile = std::unordered_map<sds, File*, SdsHasher, SdsEqual>;

  static TransferServer *Create(StoreConfig *conf);

  void Start();
  void Stop();
  int Cron();

  bool AppendMessage(const BufferedMessage &msg);
  bool PutBufferMessage(const sds &date, const sds &topic, const sds &content);

 private:
  TransferServer();

  std::mutex buffer_queue_lock_;
  std::deque<BufferedMessage> buffer_message_queue_;

  int period;
  int msg_per_cron;
  pthread_t transfer_tid_;
  aeEventLoop *el;
  long long clean_cron_id;
  std::string path_;
  std::string name_;
  int rotate_interval_;
};

#endif /* __KIDS_TRANSFERSERVER_H */
