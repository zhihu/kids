#include "monitor.h"
#include "kids.h"
#include <cassert>
#include <cctype>
#include <numeric>
#include <algorithm>
#include <string>

static void *MonitorMain(void *args) {
  aeMain(static_cast<aeEventLoop *>(args));
  return nullptr;
}

static int Cron(struct aeEventLoop *el, long long id, void *clientData) {
  NOTUSED(el);
  NOTUSED(id);
  Monitor *p = static_cast<Monitor *>(clientData);
  p->Cron();
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
  splock.Lock();
  topic_stats[local_topic].inflow_count += 1;
  topic_stats[local_topic].inflow_clients.insert(fd);
  splock.Unlock();
}

void Monitor::Stats::IncreaseTopicOutflowCount(const sds topic, int fd) {
  sds local_topic = LocalizeTopic(topic);
  splock.Lock();
  topic_stats[local_topic].outflow_count += 1;
  topic_stats[local_topic].outflow_clients.insert(fd);
  splock.Unlock();
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
}

void Monitor::Stop() {
  eventl_->stop = 1;
  pthread_join(monitor_thread_, nullptr);
}

void Monitor::Cron() {
  topic_lock_.Lock();
  CollectStats();
  topic_lock_.Unlock();
}

void Monitor::CollectStats() {
  TopicStats topic_stats;

  for (auto worker : workers_) {
    decltype(worker->stats.topic_stats) worker_stats;

    worker->stats.splock.Lock();
    worker_stats.swap(worker->stats.topic_stats);
    worker->stats.splock.Unlock();

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
  topic_lock_.Lock();
  TopicSet active_topic;
  for (auto &topic : topic_stats_)
    active_topic.insert(topic.first);
  topic_lock_.Unlock();
  return active_topic;
}

Monitor::TopicSet Monitor::GetAllTopics() {
  topic_lock_.Lock();
  auto all_topic = topic_table_;
  topic_lock_.Unlock();
  return all_topic;
}

Monitor::TopicStats Monitor::GetTopicStats() {
  topic_lock_.Lock();
  auto topic_stats = topic_stats_;
  topic_lock_.Unlock();
  return topic_stats;
}

void Monitor::RegisterClient(int fd, const char *host, int port) {
  host_lock_.Lock();
  clients_[fd] = { std::string(host), port };
  host_lock_.Unlock();
}

void Monitor::UnRegisterClient(int fd) {
  host_lock_.Lock();
  clients_.erase(fd);
  host_lock_.Unlock();
}

bool Monitor::GetTopicCount(const sds topic, Monitor::TopicCount *topic_count) {
  bool found = false;
  topic_lock_.Lock();
  auto itr = topic_stats_.find(topic);
  if (itr != topic_stats_.end()) {
    found = true;
    *topic_count = itr->second;
  }
  topic_lock_.Unlock();
  return found;
}

std::vector<Monitor::ClientAddress> Monitor::GetPublisher(const sds topic) {
  topic_lock_.Lock();
  auto inflow = topic_stats_[topic].inflow_clients;
  topic_lock_.Unlock();
  std::vector<Monitor::ClientAddress> res;
  host_lock_.Lock();
  for (auto fd : inflow) {
    res.push_back(clients_[fd]);
  }
  host_lock_.Unlock();
  return res;
}

std::vector<Monitor::ClientAddress> Monitor::GetSubscriber(const sds topic) {
  topic_lock_.Lock();
  auto outflow = topic_stats_[topic].outflow_clients;
  topic_lock_.Unlock();
  std::vector<Monitor::ClientAddress> res;
  host_lock_.Lock();
  for (auto fd : outflow) {
    res.push_back(clients_[fd]);
  }
  host_lock_.Unlock();
  return res;
}
