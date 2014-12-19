#ifndef __KIDS_STORE_STORE_H_
#define __KIDS_STORE_STORE_H_

#include <deque>
#include <string>
#include <vector>

struct Message;
struct StoreConfig;
struct Statistic;
struct aeEventLoop;

class Store {
 public:
  enum StoreState {
    Free = 0,
    Sending,
  };

 public:
  static Store *Create(StoreConfig *conf, struct Statistic *stat, aeEventLoop *el);

  explicit Store(StoreConfig *conf);
  virtual ~Store() {}

  virtual bool Open() = 0;
  virtual bool IsOpen() = 0;
  virtual void Close() = 0;

  virtual bool AddMessage(const Message *msg);

  virtual bool HaveOldMessage() { return false; }
  virtual StoreState State() { return Free; }
  virtual int GetOldestMessages(std::deque<const Message*> *msgs) { return 0; }
  virtual void DeleteOldestMessages() {}

  virtual void Cron() {}

 protected:
  enum BufferType {
    kNone = 0,
    kPrimary,
    kSecondary,
  };
  BufferType buffer_type_;

 private:
  virtual bool DoAddMessage(const Message *msg) = 0;
  bool PreAddMessage(const Message *msg);

  std::string topic_;
};

class NullStore : public Store {
 public:
  explicit NullStore(StoreConfig *conf) : Store(conf), is_open_(false) {}

  virtual bool Open() { is_open_ = true; return true; }
  virtual bool IsOpen() { return is_open_; }
  virtual void Close() { is_open_ = false; }

 private:
  virtual bool DoAddMessage(const Message *msg) { return true; }

  bool is_open_;
};

#endif  // __KIDS_STORE_STORE_H_
