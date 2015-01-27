#include "kids.h"
#include "filestore.h"

FileStore::FileStore(StoreConfig *conf, struct Statistic* stat) : Store(conf), stat_(stat) {
  is_open_ = false;
  secondary_file_ = NULL;
  path_ = conf->path;
  if (path_.back() != '/') {
    path_ = path_ + "/";
  }
  name_ = conf->name;
  LogDebug("rotate %s", conf->rotate.c_str());
  if (conf->rotate == "daily") {
    rotate_interval_ = 3600 * 24;
  } else if (conf->rotate == "hourly") {
    rotate_interval_ = 3600;
  } else if (!ParseTime(conf->rotate.c_str(), &rotate_interval_)) {
      rotate_interval_ = -1;
  }

  // if is secondary store and rotate is not specified
  // we rotate it every 10 min
  if (buffer_type_ == kSecondary && conf->rotate.empty()) {
    rotate_interval_ = 60 * 10;
  }

  // no need to flush fwrite for secondary store
  if (buffer_type_ == kSecondary || !ParseTime(conf->flush.c_str(), &flush_interval_)) {
    flush_interval_ = -1;
  }

  LogDebug("rotate interval %ds", rotate_interval_);
  LogDebug("flush interval %ds", flush_interval_);
}

FileStore::~FileStore() {
  Close();
}

bool FileStore::Open() {
  if (is_open_) Close();
  is_open_ = true;
  LogDebug("openning FileStore...");
  if (buffer_type_ == kSecondary) {
    secondary_file_ = File::Open(path_, name_, true, "kids.buffer", 0);
    if (secondary_file_ == NULL) {
      LogError(ERR_OPEN_FILE);
      is_open_ = false;
    }
    LogDebug("open result: %d", is_open_);
  } else {
    std::string root = FindRoot(path_);
    if (!MakePath(root.c_str(), 0755)) {
      is_open_ = false;
      LogError("%s of %s", ERR_CREATE_DIR, root.c_str());
    }
  }

  last_flush_ = last_rotate_ = time(NULL);

  return is_open_;
}

bool FileStore::IsOpen() {
  return is_open_;
}

bool FileStore::Flush() {
  bool success = true;
  for (auto &it : topic_file_) {
    if (!(it.second->Flush())) {
      LogError("failed to flush topic [%s]", it.first);
      success = false;
    }
  }
  return success;
}

void FileStore::Close() {
  if (!is_open_) return;
  LogDebug("closing FileStore...");
  if (buffer_type_ == kSecondary) {
    if (secondary_file_) secondary_file_->Close(true);
    delete secondary_file_;
    secondary_file_ = NULL;
  } else {
    for (auto &it : topic_file_) {
      it.second->Close(true);
      delete it.second;
      sdsfree(it.first);
    }
    topic_file_.clear();
  }
  is_open_ = false;
}

bool FileStore::DoAddMessage(const Message *msg) {
  bool success = false;
  if (buffer_type_ == kSecondary && secondary_file_ != NULL) {
    // secondary file store, write size and whole topic and msg
    success = secondary_file_->Write(msg->topic, sdslen(msg->topic), true, false)
        && secondary_file_->Write(msg->content, sdslen(msg->content), true, false);
  } else {
    // primary file store, write message to topic file
    TopicFile::iterator it = topic_file_.find(msg->topic);
    if (it != topic_file_.end()) {
      success = it->second->Write(msg->content, sdslen(msg->content), false, true);
    } else {
      time_t t = unixtime;
      if (rotate_interval_ > 0) t -= t % rotate_interval_;
      File *file = File::Open(path_, name_, false, msg->topic, t);
      if (file == NULL) {
        LogError("%s path: %s name: %s topic: %s", ERR_CREATE_FILE, path_.c_str(), name_.c_str(), msg->topic);
        success = false;
      } else {
        sds topic = sdsdup(msg->topic);
        topic_file_[topic] = file;
        success = file->Write(msg->content, sdslen(msg->content), false, true);
      }
    }
  }

  if (success) {
    if (buffer_type_ == kSecondary) {
      LogDebug("bufferred a msg, total: %d", stat_->msg_buffer);
      stat_->msg_buffer++;
      stat_->msg_buffer_size += sdslen(msg->content);
    } else {
      stat_->msg_store++;
      stat_->msg_store_size += sdslen(msg->content);
    }
  }

  return success;
}

void FileStore::Cron() {
  if (!is_open_) return;
  time_t t = unixtime;
  if (rotate_interval_ != -1 && t % rotate_interval_ == 0 && last_rotate_ < t) {
    LogDebug("rotate %d %d %d", t, last_rotate_, rotate_interval_);
    Open();
  } else if (flush_interval_ != -1 && t - last_flush_ >= flush_interval_) {
    LogDebug("flush %d %d %d", t, flush_interval_, flush_interval_);
    if (Flush()) {
      LogDebug("successfully flushed all topic");
    }
    last_flush_ = t;
  }
}

bool FileStore::HaveOldMessage() {
  std::string filename = FindOldestFile(path_.c_str());
  return !filename.empty();
}

int FileStore::GetOldestMessages(std::deque<const Message*> *msgs) {
  int size, cnt = 0;

  std::string filename = FindOldestFile(path_.c_str());
  if (filename.empty()) return 0;

  LogDebug("get old msg from %s", filename.c_str());

  File *file = File::Open(filename.c_str(), "rb");
  if (file == NULL) return 0;
  while (file->Read(&size, sizeof(int))) {
    sds topic = NULL;
    sds content = NULL;

    topic = sdsnewlen(NULL, size);
    if (!topic || !file->Read(topic, size)) goto ERR;

    if (!file->Read(&size, sizeof(int))) goto ERR;
    content = sdsnewlen(NULL, size);

    if (!content || !file->Read(content, size)) goto ERR;

    msgs->push_back(new Message(topic, content));
    cnt++;
    continue;
ERR:
    LogError("bad buffer file");
    sdsfree(topic);
    sdsfree(content);
    break;
  }

  delete file;
  return cnt;
}

void FileStore::DeleteOldestMessages() {
  LogDebug("delete oldest msg from %s", path_.c_str());
  DeleteOldestFile(path_.c_str());
}
