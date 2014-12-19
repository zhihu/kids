#ifndef __KIDS_WORKER_H_
#define __KIDS_WORKER_H_

#include <pthread.h>
#include <deque>
#include <list>
#include <unordered_map>

#include "ae/libae.h"
#include "common.h"

struct MessageQueueItem;
typedef struct MessageQueueItem MQItem;

struct MessageQueueCursor;
typedef struct MessageQueueCursor MQCursor;

struct Pattern;
class Client;

class Worker {
 public:
  static Worker* Create(const int worker_id, MQCursor *cursor, const int num_workers);
  ~Worker();

  void PutNewConnection(const int sfd);
  void GetNewConnection(const int fd);

  void AddNotify(const int worker_id);
  void NotifyNewMessage(const int worker_id);
  void PublishMessage(const int fd);
  MQItem* GetCursorPosition();

  void Cron();
  void Start();
  void Stop();

  void FreeClientAsync(Client *c);
  void FreeClient(Client *c);

  aeEventLoop *eventl_;
  Statistic stat_;
  const int worker_id_;

  std::list<Pattern*> pubsub_patterns_;
  std::unordered_map< sds, std::list<Client*>, SdsHasher, SdsEqual> clients_by_topic_;

 private:
  Worker(const int worker_id, MQCursor *cursor, const int num_workers);
  void FreeClientsInAsyncFreeQueue();

  pthread_mutex_t conn_queue_lock_;
  std::deque<int> conn_queue_;
  int conn_wait_to_notify_;
  int conn_notify_receive_fd_;
  int conn_notify_send_fd_;

  std::list<Client*> clients_;

  const int num_workers_;
  int* msg_wait_to_notify_;
  int* msg_notify_send_fd_;
  int* msg_notify_receive_fd_;

  MQCursor *cursor_;  /* this worker's position in global message queue */

  std::list<Client*> clients_to_close_;

  long long timeid_;
  pthread_t tid_;
};

#endif  // __KIDS_WORKER_H_
