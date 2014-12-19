#include "multiplestore.h"
#include "kids.h"

MultipleStore::MultipleStore(StoreConfig *conf, struct Statistic *stat, aeEventLoop *el) : Store(conf) {
  is_open_ = false;
  for (auto it : conf->stores) {
    Store *store = Store::Create(it, stat, el);
    if (store == NULL) {
      LogError(ERR_CREATE_STORE);
    } else {
      stores_.push_back(store);
    }
  }

  if (conf->success == "all") {
    success_ = kAll;
  } else {
    success_ = kAny;
  }
}

MultipleStore::~MultipleStore() {
  Close();
  for (auto it : stores_) {
    delete it;
  }
}

Store::StoreState MultipleStore::State() {
  for (auto it : stores_) {
    if (it->State() == Store::Sending) return Store::Sending;
  }
  return Store::Free;
}

bool MultipleStore::Open() {
  if (is_open_) Close();
  if (success_ == kAny) is_open_ = false;
  if (success_ == kAll) is_open_ = true;
  for (auto it : stores_) {
    bool o = it->Open();
    if (success_ == kAny && o) is_open_ = true;
    if (success_ == kAll && !o) is_open_ = false;
  }

  LogDebug("open result: %d", is_open_);
  return is_open_;
}

bool MultipleStore::IsOpen() {
  return is_open_;
}

void MultipleStore::Close() {
  if (!is_open_) return;
  LogDebug("closing MultipleStore store...");
  for (auto it : stores_) {
    it->Close();
  }
  is_open_ = false;
}

bool MultipleStore::DoAddMessage(const Message *msg) {
  int count = 0;
  bool success = true;
  for (auto it : stores_) {
    if (it->AddMessage(msg)) count++;
  }

  if (success_ == kAny) {
    success = (count > 0);
  } else {
    success = (count == stores_.size());
  }

  return success;
}

void MultipleStore::Cron() {
  if (!is_open_) return;
  for (auto it : stores_) {
    it->Cron();
  }
}
