#ifndef __KIDS_STORE_MULTIPLESTORE_H_
#define __KIDS_STORE_MULTIPLESTORE_H_

#include <string>
#include <vector>
#include "store.h"

class MultipleStore : public Store {
 public:
  MultipleStore(StoreConfig *conf, struct Statistic *stat, aeEventLoop *el);
  ~MultipleStore();

  virtual bool Open();
  virtual bool IsOpen();
  virtual void Close();

  virtual StoreState State();
  virtual void Cron();

 private:
  enum SuccessType {
    kAny = 0,
    kAll,
  };

  MultipleStore(const MultipleStore& rhs) = delete;

  virtual bool DoAddMessage(const Message *msg);

  std::vector<Store*> stores_;
  SuccessType success_;
  bool is_open_;
};

#endif  // __KIDS_STORE_MULTIPLESTORE_H_
