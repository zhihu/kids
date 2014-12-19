#ifndef __KIDS_STORER_H_
#define __KIDS_STORER_H_

#include <pthread.h>
#include <deque>

#include "ae/libae.h"
#include "common.h"

struct Message;
struct StoreConfig;

struct MessageQueueItem;
typedef struct MessageQueueItem MQItem;

struct MessageQueueCursor;
typedef struct MessageQueueCursor MQCursor;

class Store;

class Storer {
 public:
  static Storer *Create(StoreConfig *conf, MQCursor *cursor, const int num_workers);
  ~Storer();

  void AddNotify(const int worker_id);
  void NotifyNewMessage(const int worker_id);
  void StoreMessage(const int fd);
  MQItem* GetCursorPosition();

  void Cron();
  void Start();
  void Stop();

  Statistic stat_;
  aeEventLoop *eventl_;

 private:
  Storer(MQCursor* cursor, const int num_workers);

  MQCursor *cursor_;

  const int num_workers_;
  int *msg_wait_to_notify_;
  int *msg_notify_send_fd_;
  int *msg_notify_receive_fd_;

  Store *store_;
  long long timeid_;
  pthread_t tid_;
};

#endif  // __KIDS_STORER_H_
