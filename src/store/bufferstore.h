#ifndef __KIDS_STORE_BUFFERSTORE_H_
#define __KIDS_STORE_BUFFERSTORE_H_

#include <deque>

#include "store.h"

class BufferStore : public Store {
 public:
  BufferStore(StoreConfig *conf, struct Statistic *stat, aeEventLoop *el);
  ~BufferStore();

  virtual bool Open();
  virtual bool IsOpen();
  virtual void Close();

  virtual StoreState State();

  virtual void Cron();

 private:
  enum Status {
    kToPrimary = 0,
    kToSecondary,
    kTransfering,
  };

  BufferStore(const BufferStore& rhs);

  virtual bool DoAddMessage(const Message *msg);

  struct Statistic *stat_;
  Store *primary_;
  Store *secondary_;
  Status state_;
  std::deque<const Message*> *msg_to_transfer_;
  bool is_open_;
  int max_msg_per_cron_;
};

#endif  // __KIDS_STORE_BUFFERSTORE_H_
