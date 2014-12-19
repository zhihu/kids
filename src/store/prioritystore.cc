#include "kids.h"
#include "prioritystore.h"

PriorityStore::PriorityStore(StoreConfig *conf, struct Statistic *stat, aeEventLoop *el) : Store(conf) {
  is_open_ = false;
  LogDebug("rotate %s", conf->rotate.c_str());
  for (auto it : conf->stores) {
    Store *store = Store::Create(it, stat, el);
    if (store == NULL) {
      LogError(ERR_CREATE_STORE);
    } else {
      stores_.push_back(store);
    }
  }
}

PriorityStore::~PriorityStore() {
  Close();
  for (auto it : stores_) {
    delete it;
  }
}

Store::StoreState PriorityStore::State() {
  for (auto it : stores_) {
    if (it->State() == Store::Sending) return Store::Sending;
  }
  return Store::Free;
}

bool PriorityStore::Open() {
  if (is_open_) Close();
  for (auto it : stores_) {
    it->Open();
  }
  is_open_ = true;

  LogDebug("open result: %d", is_open_);
  return is_open_;
}

bool PriorityStore::IsOpen() {
  return is_open_;
}

void PriorityStore::Close() {
  if (!is_open_) return;
  LogDebug("closing PriorityStore store...");
  for (auto it : stores_) {
    it->Close();
  }
  is_open_ = false;
}

bool PriorityStore::DoAddMessage(const Message *msg) {
  for (auto it : stores_) {
    if (it->AddMessage(msg)) return true;
  }

  return false;
}

void PriorityStore::Cron() {
  if (!is_open_) return;
  for (auto it : stores_) {
    it->Cron();
  }
}
