#include "kids.h"
#include "bufferstore.h"

BufferStore::BufferStore(StoreConfig *conf, struct Statistic *stat, aeEventLoop *el) : Store(conf) {
  stat_ = stat;
  is_open_ = false;
  max_msg_per_cron_ = 100;
  primary_ = Store::Create(conf->stores[0], stat, el);
  secondary_ = Store::Create(conf->stores[1], stat, el);
  msg_to_transfer_ = NULL;
}

BufferStore::~BufferStore() {
  Close();
  delete primary_;
  delete secondary_;
}

Store::StoreState BufferStore::State() {
  if (state_ == kToPrimary) {
    return primary_->State();
  } else {
    return secondary_->State();
  }
}

bool BufferStore::Open() {
  if (is_open_) Close();
  is_open_ = true;
  if (primary_->Open()) {
    if (secondary_->HaveOldMessage()) {
      state_ = kTransfering;
      msg_to_transfer_ = new std::deque<const Message*>();
    } else {
      state_ = kToPrimary;
    }
  } else if (secondary_->Open()) {
    state_ = kToSecondary;
  } else {
    is_open_ = false;
  }

  LogDebug("open BufferStore %s", is_open_? "success" : "failed");
  return is_open_;
}

bool BufferStore::IsOpen() {
  return is_open_;
}

void BufferStore::Close() {
  if (!is_open_) return;
  LogDebug("closing BufferStore...");
  primary_->Close();
  secondary_->Close();

  if (msg_to_transfer_ != NULL && msg_to_transfer_->size() > 0) {
    LogWarning("close store, delete %d bufferred msg", msg_to_transfer_->size());
    while (!msg_to_transfer_->empty()) {
      delete msg_to_transfer_->front();
      msg_to_transfer_->pop_front();

      stat_->msg_drop++;
    }
    delete msg_to_transfer_;
  }
  is_open_ = false;
}

bool BufferStore::DoAddMessage(const Message *msg) {
  if (state_ == kToPrimary) LogDebug("state: kToPrimary");
  if (state_ == kToSecondary) LogDebug("state: kSecondary");
  if (state_ == kTransfering) LogDebug("state: kTransfering");

  if (state_ == kToPrimary) {
    if (!primary_->IsOpen() || !primary_->AddMessage(msg)) {
      LogInfo("Store to primary store faild, switch to secondary");
      secondary_->Open();
      state_ = kToSecondary;
    }
  }

  if (state_ == kToSecondary || state_ == kTransfering) {
    LogDebug("store to secondary store");
    if (!secondary_->IsOpen()) secondary_->Open();
    if (!secondary_->AddMessage(msg)) {
      LogError("Drop message: %s", ERR_SEND_SECONDARY);

      stat_->msg_drop++;
      return false;
    }
  }
  return true;
}

void BufferStore::Cron() {
  if (!is_open_) return;
  if (state_ == kToPrimary && secondary_->IsOpen()) LogWarning("secondary open when store to primary");
  if (state_ == kToSecondary) {
    if (primary_->IsOpen()) {
      LogInfo("Primary store opened, switch to transfering");
      state_ = kTransfering;
      msg_to_transfer_ = new std::deque<const Message*>();
    }
  }

  if (state_ == kTransfering) {
    // send memory bufferred buffer first
    int cnt = 0;
    while (!msg_to_transfer_->empty() && cnt < max_msg_per_cron_) {
      const Message *msg = msg_to_transfer_->front();
      msg_to_transfer_->pop_front();
      cnt++;
      if (primary_->AddMessage(msg)) {
        LogDebug("Transfer a bufferred msg, queue: %d remains total %d", msg_to_transfer_->size(), stat_->msg_buffer);
      } else {
        LogError("Transfer a bufferred message failed");
        stat_->msg_buffer--;
        delete msg;
        break;
      }
      stat_->msg_buffer--;
      delete msg;
    }

    // read a file bufferred buffer
    if (msg_to_transfer_->empty() && secondary_->HaveOldMessage()) {
      secondary_->Close();  // stop further append to current logfile (if it is the only buffer file or not)
      secondary_->GetOldestMessages(msg_to_transfer_);
      secondary_->DeleteOldestMessages();
      LogDebug("Read one buffer file");
    }

    // if no buffer remains and msg_to_transfer_ is 0
    if (msg_to_transfer_->empty() && !secondary_->HaveOldMessage()) {
      state_ = kToPrimary;
      secondary_->Close();
      delete msg_to_transfer_;
      msg_to_transfer_ = NULL;
      LogInfo("No buffer remains to transfer, switch to primary");
    }
  }

  primary_->Cron();
  secondary_->Cron();
}
