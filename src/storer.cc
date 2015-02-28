#include "kids.h"

static int StorerCron(struct aeEventLoop *el, long long id, void *clientData) {
  Storer *t = static_cast<Storer *>(clientData);
  t->Cron();
  return 100;
}

static void *StorerMain(void *arg) {
  aeMain(static_cast<aeEventLoop *>(arg));
  return NULL;
}

static void NewMessageHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
  Storer *q = static_cast<Storer *>(privdata);
  q->StoreMessage(fd);
}

Storer::Storer(StoreConfig *conf, MQCursor *cursor, const int num_workers) : cursor_(cursor), num_workers_(num_workers), conf_loaded_(ATOMIC_FLAG_INIT), conf_(conf) {
  eventl_ = aeCreateEventLoop(655350);

  msg_wait_to_notify_ = new int[num_workers_];
  msg_notify_send_fd_ = new int[num_workers_];
  msg_notify_receive_fd_ = new int[num_workers_];

  int noti_socks[2];
  for (int i = 0; i < num_workers_; ++i) {
    msg_wait_to_notify_[i] = 0;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, noti_socks)) {
      LogFatal("create thread failed: create notification socketpair failed: %s", strerror(errno));
    }

    anetNonBlock(NULL, noti_socks[0]);
    anetNonBlock(NULL, noti_socks[1]);

    msg_notify_send_fd_[i] = noti_socks[0];
    msg_notify_receive_fd_[i] = noti_socks[1];
  }
}

Storer *Storer::Create(StoreConfig *conf, MQCursor *cursor, const int num_workers) {
  Storer *q = new Storer(conf, cursor, num_workers);

  q->store_ = Store::Create(conf, &q->stat_, q->eventl_);
  if (q->store_ == NULL) {
    LogFatal(ERR_INIT_STORE);
  }

  q->store_->Open();
  std::atomic_flag_test_and_set(&q->conf_loaded_);

  for (int i = 0; i < q->num_workers_; ++i) {
    if (aeCreateFileEvent(q->eventl_, q->msg_notify_receive_fd_[i], AE_READABLE, NewMessageHandler, q) == AE_ERR) {
      LogFatal("register NewMessageHandler failed: %d, %s", q->msg_notify_receive_fd_[i], strerror(errno));
    }
  }

  q->timeid_ = aeCreateTimeEvent(q->eventl_, 1, StorerCron, q, NULL);
  return q;
}

Storer::~Storer() {
  aeDeleteTimeEvent(eventl_, timeid_);
  store_->Close();
  delete store_;

  for (int i = 0; i < num_workers_; ++i) {
    close(msg_notify_send_fd_[i]);
    close(msg_notify_receive_fd_[i]);
  }

  delete[] msg_notify_receive_fd_;
  delete[] msg_notify_send_fd_;
  delete[] msg_wait_to_notify_;

  aeDeleteEventLoop(eventl_);
  MQFreeCursor(cursor_);
}

void Storer::AddNotify(const int worker_id) {
  msg_wait_to_notify_[worker_id]++;
}

/* called by other workers to notify that new message has arrives */
void Storer::NotifyNewMessage(const int worker_id) {
  while (msg_wait_to_notify_[worker_id] > 0) {
    if (write(msg_notify_send_fd_[worker_id], "", 1) != 1) {
      break;
    }
    msg_wait_to_notify_[worker_id]--;
  }
}

/* called by master to refresh storer's config */
void Storer::RefreshConfig(StoreConfig *conf) {
  conf_ = conf;
  conf_loaded_.clear(std::memory_order_release);
}

bool Storer::IfConfigRefreshed() {
  return !conf_loaded_.test_and_set(std::memory_order_acquire);
}

void Storer::StoreMessage(const int fd) {
  if (store_->State() == Store::Free) {
    char tmp;
    if (read(fd, &tmp, 1) != 1) {
      LogError("storer reads new connection notice failed");
      return;
    }

    Message *msg = MQNext(cursor_);
    if (msg == NULL) {
      LogError("wrong notice: no new message to store");
      return;
    }

    if (!store_->AddMessage(msg)) {
      stat_.msg_drop++;
    }
  }
}

MQItem* Storer::GetCursorPosition() {
  return cursor_->item;
}

void Storer::Cron() {
  unixtime = time(NULL);
  store_->Cron();
  if (IfConfigRefreshed()) {
    DoConfigReload();
  }
}

void Storer::Start() {
  pthread_create(&tid_, NULL, StorerMain, eventl_);
}

void Storer::Stop() {
  eventl_->stop = 1;
  void *ret;
  pthread_join(tid_, &ret);
}

void Storer::DoConfigReload() {
  LogInfo("start store config reloading");

  delete store_;

  store_ = Store::Create(conf_, &stat_, eventl_);
  store_->Open();
}
