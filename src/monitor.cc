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

void Monitor::UpdateTopicTable() {
  TopicCount topic_count;
  TopicSet active_topics;

  time_t start = time(nullptr);
  for (auto worker : workers_) {
    decltype(worker->stats.topic_count) count;

    worker->stats.splock.Lock();
    count.swap(worker->stats.topic_count);
    worker->stats.splock.Unlock();

    for (auto &topic : count) {
      auto itr = topic_table_.find(topic.first);
      if (itr != topic_table_.end()) {  // fount it
        topic_count[topic.first].Add(topic.second);
        active_topics.insert(*itr);
      } else {
        sds topic_dup = sdsdup(topic.first);
        topic_table_.insert(topic_dup);
        topic_count_.emplace(topic_dup, topic.second);
        active_topics.insert(topic_dup);
      }
    }
  }

  auto time_gap = time(nullptr) - start;
  time_gap = (time_gap == 0 ? 1 : time_gap);

  for (auto &topic : topic_count) {
    topic.second.in /= time_gap;
    topic.second.out /= time_gap;
  }

  /* rvalue reference in c++ 11 */
  topic_count_.swap(topic_count);
  active_topics_.swap(active_topics);
}

Monitor::TopicSet Monitor::GetActiveTopics() {
  topic_lock.Lock();
  auto active_topic = active_topics_;
  topic_lock.Unlock();
  return active_topic;
}

Monitor::TopicSet Monitor::GetAllTopics() {
  topic_lock.Lock();
  auto all_topic = topic_table_;
  topic_lock.Unlock();
  return all_topic;
}

Monitor::TopicCount Monitor::GetTopicCount() {
  topic_lock.Lock();
  auto topic_count = topic_count_;
  topic_lock.Unlock();
  return topic_count;
}

void Monitor::Cron() {
  topic_lock.Lock();
  UpdateTopicTable();
  topic_lock.Unlock();
}
