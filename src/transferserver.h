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

class TransferServer {
 public:
  static const int kMaxMessagePerCron = 100;
  static const int kInitCronPeriod    = 5000;
  static const int kMaxCronPeriod     = 600000;

  static TransferServer *Create(StoreConfig *conf);
  ~TransferServer();

  struct BufferedMessage {
    sds timestamp;
    sds topic;
    sds content;
  };

  void Start();
  void Stop();
  int Cron();

  bool AppendMessage(const BufferedMessage &msg);
  bool PutBufferMessage(const sds &date, const sds &topic, const sds &content);

 private:
  TransferServer();

  std::mutex buffer_queue_lock_;
  std::deque<BufferedMessage> buffer_message_queue_;

  pthread_t transfer_tid_;
  aeEventLoop *el_;
  long long clean_cron_id_;
  std::string path_;
  std::string name_;
  int rotate_interval_;
};

#endif /* __KIDS_TRANSFERSERVER_H */
