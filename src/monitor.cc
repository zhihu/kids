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
  UpdateTopicTable();
  topic_lock_.Unlock();
}

void Monitor::UpdateTopicTable() {
  TopicCount topic_count;
  TopicSet active_topics;
  FdCount fds_by_topic;


  for (auto worker : workers_) {
    decltype(worker->stats.topic_count) count;
    decltype(worker->stats.fds_by_topic) fds;

    worker->stats.splock.Lock();
    count.swap(worker->stats.topic_count);
    fds.swap(worker->stats.fds_by_topic);
    worker->stats.splock.Unlock();

    for (auto &topic : count) {
      auto itr = topic_table_.find(topic.first);
      sds topic_dup;
      if (itr != topic_table_.end()) {  // fount it
        topic_count[topic.first].Add(topic.second);
        topic_dup = *itr;
      } else {
        topic_dup = sdsdup(topic.first);
        topic_table_.insert(topic_dup);
        topic_count_.emplace(topic_dup, topic.second);
      }
      active_topics.insert(topic_dup);
      fds_by_topic[topic_dup].inflow.insert(fds[topic_dup].inflow.begin(),
                                            fds[topic_dup].inflow.end());
      fds_by_topic[topic_dup].outflow.insert(fds[topic_dup].outflow.begin(),
                                             fds[topic_dup].outflow.end());
    }
  }

  for (auto &topic : topic_count) {
    topic.second.in /= kCronPeriod;
    topic.second.out /= kCronPeriod;
  }

  /* rvalue reference in c++ 11 */
  fds_by_topic_.swap(fds_by_topic);
  topic_count_.swap(topic_count);
  active_topics_.swap(active_topics);
}

Monitor::TopicSet Monitor::GetActiveTopics() {
  topic_lock_.Lock();
  auto active_topic = active_topics_;
  topic_lock_.Unlock();
  return active_topic;
}

Monitor::TopicSet Monitor::GetAllTopics() {
  topic_lock_.Lock();
  auto all_topic = topic_table_;
  topic_lock_.Unlock();
  return all_topic;
}

Monitor::TopicCount Monitor::GetTopicCount() {
  topic_lock_.Lock();
  auto topic_count = topic_count_;
  topic_lock_.Unlock();
  return topic_count;
}

void Monitor::RegisterClient(int fd, const char *host, int port) {
  host_lock_.Lock();
  clients_[fd] = { std::string(host), port };
  host_lock_.Unlock();
}

void Monitor::UnRegisterClient(int fd) {
  host_lock_.Lock();
  auto itr = clients_.find(fd);
  if (itr != clients_.end()) {
    clients_.erase(fd);
  }
  host_lock_.Unlock();
}

std::vector<Monitor::ClientAddress> Monitor::GetInflowClients(const sds topic) {
  topic_lock_.Lock();
  auto in = fds_by_topic_[topic].inflow;
  topic_lock_.Unlock();
  std::vector<Monitor::ClientAddress> res;
  host_lock_.Lock();
  for (auto fd : in) {
    res.push_back(clients_[fd]);
  }
  host_lock_.Unlock();
  return res;
}
std::vector<Monitor::ClientAddress> Monitor::GetOutflowClients(const sds topic) {
  topic_lock_.Lock();
  auto out = fds_by_topic_[topic].outflow;
  topic_lock_.Unlock();
  std::vector<Monitor::ClientAddress> res;
  host_lock_.Lock();
  for (auto fd : out) {
    res.push_back(clients_[fd]);
  }
  host_lock_.Unlock();
  return res;
}
