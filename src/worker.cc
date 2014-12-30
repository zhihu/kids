#include <algorithm>
#include "kids.h"

extern Master *kids;

static void *WorkerMain(void *arg) {
  aeMain(static_cast<aeEventLoop *>(arg));
  return NULL;
}

// wake up at least once every 1000ms
static int WorkerCron(struct aeEventLoop *el, long long id, void *clientData) {
  Worker *t = static_cast<Worker*>(clientData);
  t->Cron();
  return 100;
}

static void NewMessageHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
  Worker *t = static_cast<Worker*>(privdata);
  t->PublishMessage(fd);
}

static void NewConnectionHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
  Worker *t = static_cast<Worker*>(privdata);
  t->GetNewConnection(fd);
}

Worker* Worker::Create(const int worker_id, MQCursor *cursor, const int num_workers) {
  Worker *t = new Worker(worker_id, cursor, num_workers);

  if (aeCreateFileEvent(t->eventl_, t->conn_notify_receive_fd_, AE_READABLE, NewConnectionHandler, t) == AE_ERR) {
    LogFatal("create notify event failed: %s", strerror(errno));
  }

  for (int i = 0; i < t->num_workers_; ++i) {
    if (aeCreateFileEvent(t->eventl_, t->msg_notify_receive_fd_[i], AE_READABLE, NewMessageHandler, t) == AE_ERR) {
      LogFatal("create notify event failed: %s", strerror(errno));
    }
  }

  t->timeid_ = aeCreateTimeEvent(t->eventl_, 1, WorkerCron, t, NULL);
  return t;
}

Worker::Worker(const int worker_id, MQCursor *cursor, const int num_workers)
    : worker_id_(worker_id), num_workers_(num_workers) {
  eventl_ = aeCreateEventLoop(655350);
  pthread_mutex_init(&conn_queue_lock_, NULL);

  msg_wait_to_notify_ = new int[num_workers_];
  msg_notify_send_fd_ = new int[num_workers_];
  msg_notify_receive_fd_ = new int[num_workers_];

  int noti_socks[2];
  for (int i = 0; i < num_workers_ + 1; ++i) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, noti_socks)) {
      LogFatal("create thread failed: create notification socketpair failed: %s", strerror(errno));
    }

    anetNonBlock(NULL, noti_socks[0]);
    anetNonBlock(NULL, noti_socks[1]);
    if (i == num_workers_) {
      conn_wait_to_notify_ = 0;
      conn_notify_send_fd_ = noti_socks[0];
      conn_notify_receive_fd_ = noti_socks[1];
    } else {
      msg_wait_to_notify_[i] = 0;
      msg_notify_send_fd_[i] = noti_socks[0];
      msg_notify_receive_fd_[i] = noti_socks[1];
    }
  }

  cursor_ = cursor;
}

Worker::~Worker() {
  aeDeleteTimeEvent(eventl_, timeid_);

  MQFreeCursor(cursor_);
  close(conn_notify_send_fd_);
  close(conn_notify_receive_fd_);
  for (int i = 0; i < num_workers_; ++i) {
    close(msg_notify_send_fd_[i]);
    close(msg_notify_receive_fd_[i]);
  }

  delete[] msg_notify_receive_fd_;
  delete[] msg_notify_send_fd_;
  delete[] msg_wait_to_notify_;

  for (auto it = clients_to_close_.begin(); it != clients_to_close_.end(); ++it) {
    delete *it;
  }

  for (auto it = clients_.begin(); it != clients_.end(); ++it) {
    delete *it;
  }

  pthread_mutex_destroy(&conn_queue_lock_);
  aeDeleteEventLoop(eventl_);
}

// called by main thread to assign a connection to this thread
void Worker::PutNewConnection(const int sfd) {
  pthread_mutex_lock(&conn_queue_lock_);
  conn_queue_.push_back(sfd);
  pthread_mutex_unlock(&conn_queue_lock_);

  conn_wait_to_notify_++;
  while (conn_wait_to_notify_ > 0) {
    if (write(conn_notify_send_fd_, "", 1) != 1) {
      break;
    }
    conn_wait_to_notify_--;
  }
}

void Worker::GetNewConnection(const int fd) {
  int sfd;
  if (read(fd, &sfd, 1) != 1) {
    LogError("read new connection notice failed");
    return;
  }

  pthread_mutex_lock(&conn_queue_lock_);
  sfd = conn_queue_.front();
  conn_queue_.pop_front();
  pthread_mutex_unlock(&conn_queue_lock_);

  Client *c = Client::Create(sfd, this);
  if (c == NULL) {
    LogError("create client failed");
    return;
  }

  clients_.push_back(c);
  LogDebug("thread %d accept a client", worker_id_);
}

void Worker::AddNotify(const int worker_id) {
  msg_wait_to_notify_[worker_id]++;
}

/* called by other workers to notify this worker that new message has arrives */
void Worker::NotifyNewMessage(const int worker_id) {
  while (msg_wait_to_notify_[worker_id] > 0) {
    if (write(msg_notify_send_fd_[worker_id], "", 1) != 1) {
      break;
    }
    msg_wait_to_notify_[worker_id]--;
  }
}

void Worker::PublishMessage(const int fd) {
  char tmp;
  if (read(fd, &tmp, 1) != 1) {
    LogError("read new connection notice failed");
    return;
  }

  Message *msg = MQNext(cursor_);
  if (msg == NULL) {
    LogError("wrong notice: no new message to publish");
    return;
  }

  const sds topic = msg->topic;
  const sds content = msg->content;

  LogDebug("Publishing msg: (%s,%s)", topic, content);

  auto clients = clients_by_topic_.find(topic);
  if (clients != clients_by_topic_.end()) {
    for (auto it : clients->second) {
      stat_.msg_out++;
      stat_.msg_out_traffic += (sdslen(topic) + sdslen(content));
      it->ReceiveMessage(topic, content);
    }
  }

  // Send to clients listening to matching channels
  for (auto it : pubsub_patterns_) {
    if (stringmatchlen(it->pattern, sdslen(it->pattern),
                       topic, sdslen(topic), kids->config_.ignore_case)) {
      stat_.msg_out++;
      stat_.msg_out_traffic += (sdslen(topic) + sdslen(content));
      it->client->ReceivePatternMessage(topic, content, it->pattern);
    }
  }
}

void Worker::Cron() {
  unixtime = time(NULL);
  FreeClientsInAsyncFreeQueue();

  kids->NotifyNewMessage(worker_id_);

  stat_.clients = clients_.size();
  stat_.topics = clients_by_topic_.size();
  stat_.patterns = pubsub_patterns_.size();
}

void Worker::Start() {
  pthread_create(&tid_, NULL, WorkerMain, eventl_);
}

void Worker::Stop() {
  eventl_->stop = 1;
  void *ret;
  pthread_join(tid_, &ret);
}

void Worker::FreeClientsInAsyncFreeQueue() {
  for (auto it : clients_to_close_) {
    delete it;
  }
  clients_to_close_.clear();
}

void Worker::FreeClientAsync(Client *c) {
  if (std::find(clients_.begin(), clients_.end(), c) != clients_.end()) {
    clients_.remove(c);
    clients_to_close_.push_back(c);
  }
}

void Worker::FreeClient(Client *c) {
  std::list<Client*>::iterator it = std::find(clients_.begin(), clients_.end(), c);
  if (it != clients_.end()) {
    Client *c = (*it);
    clients_.erase(it);
    delete c;
  }
}

MQItem* Worker::GetCursorPosition() {
  return cursor_->item;
}
