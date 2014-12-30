#include "master.h"
#include "kids.h"

void AcceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
  int port, cfd;
  char ip[128];
  char error[1024];

  cfd = anetTcpAccept(error, fd, ip, sizeof(ip), &port);
  if (cfd == AE_ERR) {
    LogError("Accept client connection failed: %s", error);
    return;
  }
  LogInfo("Accepted %s:%d", ip, port);

  if (kids->config_.monitor)
    kids->monitor_->RegisterClient(cfd, ip, port);
  kids->AssignNewConnection(cfd);
}

void AcceptUnixHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
  int cfd;
  char error[1024];

  cfd = anetUnixAccept(error, fd);
  if (cfd == AE_ERR) {
    LogWarning("Accept client connection failed: %s", error);
    return;
  }
  LogInfo("Accepted connection to %s", kids->config_.listen_socket.c_str());

  kids->AssignNewConnection(cfd);
}

static int ServerCron(struct aeEventLoop *el, long long id, void *clientData) {
  Master *k = static_cast<Master *>(clientData);
  k->Cron();
  return 100;  // cron every 100ms
}

Master::Master(const KidsConfig *conf)
    : config_ { atoi(conf->worker_threads.c_str()),
      conf->listen_socket,
      conf->listen_host,
      atoi(conf->listen_port.c_str()),
      conf->ignore_case == "on",
      conf->monitor == "on",
      {conf->nlimit[0], conf->nlimit[1], conf->nlimit[2]},
      1000000}
{
  stat_.start_time = time(NULL);

  last_thread_ = -1;
  message_queue_ = MQOpen();
  eventl_ = aeCreateEventLoop(65535);

  tcpfd_ = -1;
  unixfd_ = -1;

  char error[1024];
  int backlog = 511;
  if (config_.listen_port > 0) {
    if (config_.listen_host.empty()) {
      tcpfd_ = anetTcpServer(error, config_.listen_port, NULL, backlog);
    } else {
      tcpfd_ = anetTcpServer(error, config_.listen_port, const_cast<char*>(config_.listen_host.c_str()), backlog);
    }
    if (tcpfd_ == ANET_ERR) {
      LogError("%s", error);
    }
  }

  if (!config_.listen_socket.empty()) {
    unlink(const_cast<char*>(config_.listen_socket.c_str()));
    unixfd_ = anetUnixServer(error, const_cast<char*>(config_.listen_socket.c_str()), 0666, backlog);
    if (unixfd_ == ANET_ERR) {
      LogError("%s", error);
    }
  }
}

Master *Master::Create(const KidsConfig *conf) {
  Master *k = new Master(conf);
  if (k->config_.worker_threads == 0) {
    LogFatal("wrong config for worker_threads: 0");
  }
  kids = k;

  if (k->tcpfd_ > 0 && aeCreateFileEvent(k->eventl_, k->tcpfd_, AE_READABLE, AcceptTcpHandler, NULL) == AE_ERR) {
    LogError("oom when create file event");
  } else if (k->tcpfd_ > 0) {
    LogInfo("The server is now ready to accept connections on %s:%d",
            k->config_.listen_host.c_str(), k->config_.listen_port);
  }

  if (k->unixfd_ > 0 && aeCreateFileEvent(k->eventl_, k->unixfd_, AE_READABLE, AcceptUnixHandler, NULL) == AE_ERR) {
    LogError("oom when create file event");
  } else if (k->unixfd_ > 0) {
    LogInfo("The server is now ready to accept connections at %s", k->config_.listen_socket.c_str());
  }

  if (k->tcpfd_ == -1 && k->unixfd_ == -1) {
    LogFatal("listen failed");
  }
  MQCursor *c = MQCreateCursor(k->message_queue_);
  k->storer_ = Storer::Create(conf->store, c, k->config_.worker_threads);
  for (int i = 0; i < k->config_.worker_threads; ++i) {
    c = MQCreateCursor(k->message_queue_);
    k->workers_.push_back(Worker::Create(i, c, k->config_.worker_threads));
  }

  k->timeid_ = aeCreateTimeEvent(k->eventl_, 5000, ServerCron, k, NULL);

  if (k->config_.monitor)
    k->monitor_ = Monitor::Create(k->workers_);

  return k;
}

Master::~Master() {
  LogDebug("all eventloops have stoped");

  aeDeleteTimeEvent(eventl_, timeid_);

  for (auto it : workers_) {
    delete it;
  }
  delete storer_;

  if (config_.monitor)
    delete monitor_;

  if (unixfd_ > 0) {
    close(unixfd_);
    unlink(config_.listen_socket.c_str());
  }

  if (tcpfd_ > 0) {
    close(tcpfd_);
  }

  aeDeleteEventLoop(eventl_);
  MQClose(message_queue_);
}

void Master::AssignNewConnection(const int fd) {
  last_thread_ = (last_thread_ + 1) % config_.worker_threads;
  Worker *t = workers_[last_thread_];
  t->PutNewConnection(fd);
}

/* called by worker whose id is worker_id
 * 1. put (topic, content) in the global message queue, do not copy data
 * 2. notify all worker threads (including itself) and the store thread
 * be sure to set topic and content to NULL after calling this function
 */
bool Master::PutMessage(const sds &topic, const sds &content, const int worker_id) {
  if (MQSize(message_queue_) >= config_.queue_limit) {
    LogError("message queue full, dropping message");
    stat_.msg_drop++;
    return false;
  }

  MQPush(message_queue_, topic, content);

  for (auto it : workers_) {
    it->AddNotify(worker_id);
  }
  storer_->AddNotify(worker_id);

  NotifyNewMessage(worker_id);
  return true;
}

void Master::NotifyNewMessage(const int worker_id) {
  for (auto worker : workers_) {
    worker->NotifyNewMessage(worker_id);
  }
  storer_->NotifyNewMessage(worker_id);
}

void Master::Start() {
  storer_->Start();
  for (auto it : workers_) {
    it->Start();
  }

  if (config_.monitor)
    monitor_->Start();

  aeMain(eventl_);
}

void Master::Stop() {

  eventl_->stop = 1;
  if (config_.monitor)
    monitor_->Stop();

  for (auto it : workers_) {
    it->Stop();
  }
  storer_->Stop();
}

void Master::Cron() {
  unixtime = time(NULL);
  static uint64_t last_in = 0;
  static uint64_t last_out = 0;
  static time_t last_time = 0;

  /* garbage collection */
  MQItem **cursor_positions = new MQItem *[config_.worker_threads + 1];
  int i = 0;
  for (auto it : workers_) {
    cursor_positions[i++] = it->GetCursorPosition();
  }
  cursor_positions[i++] = storer_->GetCursorPosition();
  uint64_t msg_in_queue = MQFreeOldMessages(message_queue_, cursor_positions, i);
  delete[] cursor_positions;

  Statistic stat;
  for (auto it : workers_) {
    stat.Add(it->stat_);
  }
  stat.Add(storer_->stat_);
  /* ensure these fields are valid */
  stat.msg_in_queue = msg_in_queue;
  stat.start_time = stat_.start_time;
  stat.msg_in_ps = stat_.msg_in_ps;
  stat.msg_out_ps = stat_.msg_out_ps;
  stat.msg_drop = stat_.msg_drop;

  if (unixtime - last_time >= 5) {
    stat.msg_in_ps = (stat.msg_in - last_in) / (unixtime - last_time);
    stat.msg_out_ps = (stat.msg_out - last_out) / (unixtime - last_time);

    last_in = stat.msg_in;
    last_out = stat.msg_out;
    last_time = unixtime;
  }
  stat_ = stat;
}
