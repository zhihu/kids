#ifndef __KIDS_STORE_PRIORITYSTORE_H_
#define __KIDS_STORE_PRIORITYSTORE_H_

#include <vector>
#include "store.h"

class PriorityStore : public Store {
 public:
  PriorityStore(StoreConfig *conf, struct Statistic *stat, aeEventLoop *el);
  ~PriorityStore();

  virtual bool Open();
  virtual bool IsOpen();
  virtual void Close();

  virtual StoreState State();
  virtual void Cron();

 private:
  explicit PriorityStore(const PriorityStore& rhs) = delete;

  virtual bool DoAddMessage(const Message *msg);

  std::vector<Store*> stores_;
  bool is_open_;
};

#endif  // __KIDS_STORE_PRIORITYSTORE_H_
