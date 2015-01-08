#include <vector>
#include <algorithm>
#include "kids.h"
#include "transferserver.h"
#define NOTUSED(V) ((void *)V)

static void *TransferServerMain(void *args) {
  aeMain(static_cast<aeEventLoop *>(args));
  return nullptr;
}

static int Cron(struct aeEventLoop *el, long long id, void *clientData) {
  NOTUSED(el);
  NOTUSED(id);
  TransferServer *transfer_server = static_cast<TransferServer *>(clientData);
  return transfer_server->Cron();
}

bool TransferServer::AppendMessage(const BufferedMessage &msg) {
  bool success = false;
  int t = atoi(msg.timestamp);
  if (rotate_interval_ > 0)
    t -= t % rotate_interval_;
  File *file = File::Open(path_, name_, false, std::string(msg.topic), t);
  success = file->Write(msg.content, sdslen(msg.content), false, true);
  file->Close(true);
  delete file;

  sdsfree(msg.timestamp);
  sdsfree(msg.topic);
  sdsfree(msg.content);

  return success;
}

int TransferServer::Cron() {
  int count = 0;
  std::vector<BufferedMessage> msg_queue;
  msg_queue.reserve(kMaxMessagePerCron);

  {
    std::lock_guard<std::mutex> _(buffer_queue_lock_);
    while (count < msg_per_cron) {
      if (buffer_message_queue_.size() == 0)
        break;
      auto msg = buffer_message_queue_.front();

      buffer_message_queue_.pop_front();
      msg_queue.push_back(msg);
      count++;
    }
  }

  std::vector<BufferedMessage> fresh_msgs;
  for (auto i : msg_queue) {
    auto timestamp = atoi(i.timestamp);
    if (timestamp <= 0) {
      LogWarning("Error transfer command");
    } else if (time(nullptr) - timestamp < 4000) {
      fresh_msgs.push_back(i);
    } else {
      if (!AppendMessage(i))
        LogError("Appending buffered messge to file failed!");
    }
  }

  std::lock_guard<std::mutex> _(buffer_queue_lock_);
  for (auto i : fresh_msgs)
    buffer_message_queue_.push_back(i);
  return kInitCronPeriod;
}

void TransferServer::Stop() {
  el->stop = 1;
  pthread_join(transfer_tid_, nullptr);
}

TransferServer::TransferServer() {
  el = aeCreateEventLoop(10);
  period       = kInitCronPeriod;
  msg_per_cron = kMaxMessagePerCron;
}

bool TransferServer::PutBufferMessage(const sds &date, const sds &topic, const sds &content) {
  std::lock_guard<std::mutex> _(buffer_queue_lock_);
  buffer_message_queue_.push_back({date, topic, content});
  LogInfo("%s pushed", topic);
  return true;
}

void TransferServer::Start() {
  pthread_create(&transfer_tid_, nullptr, TransferServerMain, el);
}

TransferServer *TransferServer::Create(StoreConfig *conf) {
  TransferServer *server = new TransferServer;
  server->path_ = conf->path;
  server->name_ = conf->name;

  if (server->path_.back() != '/') {
    server->path_ += "/";
  }

  int rotate_interval_;
  if (conf->rotate == "daily") {
    rotate_interval_ = 3600 * 24;
  } else if (conf->rotate == "hourly") {
    rotate_interval_ = 3600;
  } else if (!ParseTime(conf->rotate.c_str(), &rotate_interval_)) {
    rotate_interval_ = -1;
  }

  server->rotate_interval_ = rotate_interval_;
  server->clean_cron_id = aeCreateTimeEvent(server->el, 5000, ::Cron, server, nullptr);
  return server;
}
