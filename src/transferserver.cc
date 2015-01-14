#include <vector>
#include <algorithm>
#include <memory>
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
  int timestamp = atoi(msg.timestamp);
  if (rotate_interval_ > 0)
    timestamp -= timestamp % rotate_interval_;
  auto file = std::unique_ptr<File>(File::Open(path_, name_, false, std::string(msg.topic), timestamp));
  success = file->Write(msg.content, sdslen(msg.content), false, true);

  sdsfree(msg.timestamp);
  sdsfree(msg.topic);
  sdsfree(msg.content);

  return success;
}

int TransferServer::Cron() {
  int count = 0;
  std::vector<BufferedMessage> tmp_msg_queue;
  tmp_msg_queue.reserve(kMaxMessagePerCron);

  {
    std::lock_guard<std::mutex> _(buffer_queue_lock_);
    while (count < kMaxMessagePerCron) {
      if (buffer_message_queue_.size() == 0)
        break;
      auto msg = buffer_message_queue_.front();
      buffer_message_queue_.pop_front();
      tmp_msg_queue.push_back(msg);
      count++;
    }
  }

  if (tmp_msg_queue.size() == 0) {
    return kMaxCronPeriod;
  }

  std::vector<BufferedMessage> fresh_msgs;
  for (auto i : tmp_msg_queue) {
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

  auto fresh_rate = ((double) fresh_msgs.size()) / ((double) tmp_msg_queue.size());

  if (fresh_rate <= 0.3)
    return kInitCronPeriod;
  else
    return kMaxCronPeriod * fresh_rate;
}

void TransferServer::Stop() {
  el->stop = 1;
  pthread_join(transfer_tid_, nullptr);
}

TransferServer::TransferServer() {
  el = aeCreateEventLoop(10);
}

bool TransferServer::PutBufferMessage(const sds &date, const sds &topic, const sds &content) {
  std::lock_guard<std::mutex> _(buffer_queue_lock_);
  buffer_message_queue_.push_back({date, topic, content});
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
