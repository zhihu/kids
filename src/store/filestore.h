#ifndef __KIDS_STORE_FILESTORE_H_
#define __KIDS_STORE_FILESTORE_H_

#include <deque>
#include <string>
#include <unordered_map>

#include "common.h"
#include "store.h"
#include "filesystem.h"

class FileStore : public Store {
 public:
  FileStore(StoreConfig *conf, struct Statistic *stat);
  ~FileStore();

  virtual bool Open();
  virtual bool IsOpen();

  virtual void Cron();

  virtual bool HaveOldMessage();
  virtual int GetOldestMessages(std::deque<BufferedMessage> *msgs);
  virtual void DeleteOldestMessages();

 private:
  typedef std::unordered_map<sds, File*, SdsHasher, SdsEqual> TopicFile;

  FileStore(const FileStore& rhs) = delete;

  virtual void Close();
  virtual bool DoAddMessage(const Message *msg);

  std::string path_;
  std::string name_;

  int rotate_interval_;
  time_t last_rotate_;

  File *secondary_file_;
  TopicFile topic_file_;
  bool is_open_;

  struct Statistic *stat_;
};

#endif  // __KIDS_STORE_FILESTORE_H_
