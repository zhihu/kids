#include "monitor.h"
#include "kids.h"
#include <algorithm>
#include <string>
#include <cinttypes>
#include <mutex>

static void *MonitorMain(void *args) {
  aeMain(static_cast<aeEventLoop *>(args));
  return nullptr;
}

static int Cron(struct aeEventLoop *el, long long id, void *clientData) {
  NOTUSED(el);
  NOTUSED(id);
  Monitor *monitor = static_cast<Monitor *>(clientData);
  monitor->Cron();
  return (Monitor::kCronPeriod - time(nullptr) % Monitor::kCronPeriod) * 1000;
}

void Monitor::TopicCount::Merge(const TopicCount &count) {
  inflow_count += count.inflow_count;
  outflow_count += count.outflow_count;
  inflow_clients.insert(count.inflow_clients.begin(), count.inflow_clients.end());
  outflow_clients.insert(count.outflow_clients.begin(), count.outflow_clients.end());
}

void Monitor::Stats::IncreaseTopicInflowCount(const sds topic, int fd) {
  sds local_topic = LocalizeTopic(topic);
  std::lock_guard<Spinlock> _(splock);
  topic_stats[local_topic].inflow_count += 1;
  topic_stats[local_topic].inflow_clients.insert(fd);
}

void Monitor::Stats::IncreaseTopicOutflowCount(const sds topic, int fd) {
  sds local_topic = LocalizeTopic(topic);
  std::lock_guard<Spinlock> _(splock);
  topic_stats[local_topic].outflow_count += 1;
  topic_stats[local_topic].outflow_clients.insert(fd);
}

Monitor::Stats::~Stats() {
  for (auto &topic : topic_table) {
    sdsfree(topic);
  }
}

sds Monitor::Stats::LocalizeTopic(const sds topic) {
  sds local_topic;
  auto itr = topic_table.find(topic);
  if (itr == topic_table.end()) {
    local_topic = sdsdup(topic);
    topic_table.insert(local_topic);
  } else {
    local_topic = *itr;
  }
  return local_topic;
}

Monitor *Monitor::Create(const std::vector<Worker*> &workers_) {
  Monitor *monitor = new Monitor();
  monitor->workers_ = workers_;
  monitor->cron_id_ = aeCreateTimeEvent(monitor->eventl_, kCronPeriod, ::Cron, monitor, nullptr);
  return monitor;
}

Monitor::Monitor() {
  eventl_ = aeCreateEventLoop(10);
}

Monitor::~Monitor() {
  aeDeleteTimeEvent(eventl_, cron_id_);
  aeDeleteEventLoop(eventl_);

  for (auto &topic : topic_table_) {
    sdsfree(topic);
  }
}

void Monitor::Start() {
  pthread_create(&monitor_thread_, nullptr, MonitorMain, eventl_);
  const char *options[] = { "listening_ports", "8327", "num_threads", "1",  nullptr };
  http_server_ = std::make_shared<CivetServer>(options);
  http_server_->addHandler("/api/v1/topic$", new TopicsHandler);
  http_server_->addHandler("/api/v1/topic/**$", new TopicHandler);
  LogInfo("HTTP server started on 8327");
}

void Monitor::Stop() {
  eventl_->stop = 1;
  pthread_join(monitor_thread_, nullptr);
}

void Monitor::Cron() {
  std::lock_guard<Spinlock> _(topic_lock_);
  CollectStats();
}

void Monitor::CollectStats() {
  TopicStats topic_stats;

  for (auto worker : workers_) {
    decltype(worker->stats.topic_stats) worker_stats;

    {
      std::lock_guard<Spinlock> _(worker->stats.splock);
      worker_stats.swap(worker->stats.topic_stats);
    }

    for (auto &topic : worker_stats) {
      auto itr = topic_table_.find(topic.first);
      sds local_topic;
      if (itr != topic_table_.end()) {
        local_topic = *itr;
        topic_stats[local_topic].Merge(topic.second);
      } else {
        local_topic = sdsdup(topic.first);
        topic_table_.insert(local_topic);
        topic_stats.emplace(local_topic, topic.second);
      }
    }
  }

  for (auto &topic : topic_stats) {
    topic.second.inflow_count /= kCronPeriod;
    topic.second.outflow_count /= kCronPeriod;
  }

  /* rvalue reference in C++ 11 */
  topic_stats_.swap(topic_stats);
}

Monitor::TopicSet Monitor::GetActiveTopics() {
  TopicSet active_topic;
  std::lock_guard<Spinlock> _(topic_lock_);
  for (auto &topic : topic_stats_)
    active_topic.insert(topic.first);
  return active_topic;
}

Monitor::TopicSet Monitor::GetAllTopics() {
  std::lock_guard<Spinlock> _(topic_lock_);
  return topic_table_;
}

Monitor::TopicStats Monitor::GetTopicStats() {
  std::lock_guard<Spinlock> _(topic_lock_);
  return topic_stats_;
}

void Monitor::RegisterClient(int fd, const char *host, int port) {
  std::lock_guard<Spinlock> _(host_lock_);
  clients_[fd] = { std::string(host), port };
}

void Monitor::UnRegisterClient(int fd) {
  std::lock_guard<Spinlock> _(host_lock_);
  clients_.erase(fd);
}

bool Monitor::GetTopicCount(const sds topic, Monitor::TopicCount *topic_count) {
  bool found = false;
  std::lock_guard<Spinlock> _(topic_lock_);
  auto itr = topic_stats_.find(topic);
  if (itr != topic_stats_.end()) {
    found = true;
    *topic_count = itr->second;
  }
  return found;
}

std::vector<Monitor::ClientAddress> Monitor::GetPublisher(const sds topic) {
  std::vector<Monitor::ClientAddress> res;
  std::unordered_set<int> inflow_clients;

  {
    std::lock_guard<Spinlock> _(topic_lock_);
    auto itr = topic_stats_.find(topic);
    if (itr == topic_stats_.end()) {
      return res;
    }
    inflow_clients = itr->second.inflow_clients;
  }

  {
    std::lock_guard<Spinlock> _(topic_lock_);
    for (auto fd : inflow_clients) {
      auto itr = clients_.find(fd);
      if (itr != clients_.end())
        res.push_back(itr->second);
    }
  }

  return res;
}

std::vector<Monitor::ClientAddress> Monitor::GetSubscriber(const sds topic) {
  std::vector<Monitor::ClientAddress> res;
  std::unordered_set<int> outflow_clients;

  {
    std::lock_guard<Spinlock> _(topic_lock_);
    auto itr = topic_stats_.find(topic);
    if (itr == topic_stats_.end()) {
      return res;
    }
    outflow_clients = itr->second.outflow_clients;
  }

  {
    std::lock_guard<Spinlock> _(topic_lock_);
    for (auto fd : outflow_clients) {
      auto itr = clients_.find(fd);
      if (itr != clients_.end())
        res.push_back(itr->second);
    }
  }

  return res;
}

/* HTTP server */

bool TopicsHandler::handleGet(CivetServer *server, struct mg_connection *conn) {
  std::string s = "";
  mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
  auto topic_stats = kids->monitor_->GetTopicStats();
  mg_printf(conn, "{\"topics\":[");
  for (auto itr = topic_stats.begin(); itr != topic_stats.end(); itr++) {
    mg_printf(conn, "{");
    mg_printf(conn, "\"topic\":\"%s\",", itr->first);
    mg_printf(conn, "\"msg_in_ps\":%" PRIu32 ",", itr->second.inflow_count);
    mg_printf(conn, "\"msg_out_ps\":%" PRIu32, itr->second.outflow_count);
    mg_printf(conn, "}");
    if (std::next(itr) != topic_stats.end())
      mg_printf(conn, ",");
  }
  mg_printf(conn, "]}");
  return true;
}

bool TopicHandler::handleGet(CivetServer *server, struct mg_connection *conn) {
  bool showclient = false;
  std::string param;
  mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
  if (CivetServer::getParam(conn, "showclient", param)) {
    showclient = (param == "true");
  }
  Monitor::TopicCount topic_count;
  auto *req_info = mg_get_request_info(conn);
  sds topic = sdsnew(req_info->uri + sizeof("/api/v1/topic"));

  if (kids->monitor_->GetTopicCount(topic, &topic_count)) {
    mg_printf(conn, "{\"topic\":");
    mg_printf(conn, "\"%s\",", topic);
    mg_printf(conn, "\"msg_in_ps\":%" PRIu32 ",", topic_count.inflow_count);
    mg_printf(conn, "\"msg_out_ps\":%" PRIu32, topic_count.outflow_count);
    if (showclient) {
      auto inflow_clients = kids->monitor_->GetPublisher(topic);
      auto outflow_clients = kids->monitor_->GetSubscriber(topic);
      mg_printf(conn, ",");
      mg_printf(conn, "\"inflow_clients\":[");
      for (auto itr = inflow_clients.begin(); itr != inflow_clients.end(); itr++) {
        mg_printf(conn, "{\"host\":\"%s\", \"port\":\"%d\"}", itr->host.c_str(), itr->port);
        if (std::next(itr) != inflow_clients.end())
          mg_printf(conn, ",");
      }
      mg_printf(conn, "],");
      mg_printf(conn, "\"outflow_clients\":[");
      for (auto itr = outflow_clients.begin(); itr != outflow_clients.end(); itr++) {
        mg_printf(conn, "{\"host\":\"%s\", \"port\":\"%d\"}", itr->host.c_str(), itr->port);
        if (std::next(itr) != outflow_clients.end())
          mg_printf(conn, ",");
      }
      mg_printf(conn, "]");
    }
    mg_printf(conn, "}");
  } else {
    mg_printf(conn, "{\"error\":\"%s is not active now\"}", topic);
  }
  sdsfree(topic);
  return true;
}
