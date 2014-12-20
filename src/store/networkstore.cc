#include "kids.h"
#include "networkstore.h"
void SendRequest(aeEventLoop *el, int fd, void *privdata, int mask);
void ReadReply(aeEventLoop *el, int fd, void *privdata, int mask);

class NetworkStore::Agent {
  friend void SendRequest(aeEventLoop *el, int fd, void *privdata, int mask);
  friend void ReadReply(aeEventLoop *el, int fd, void *privdata, int mask);

 public:
  static Agent *Create(const int fd, NetworkStore *store);
  ~Agent();
  std::string ToString();

  void Log(const Message *msg);
  aeEventLoop *eventl_;

 private:
  Agent() {}
  void Reset();

  bool ProcessReplyBuffer();
  bool ParseReplyBuffer();
  bool ProcessResult();

  bool SetWriteEvent();

  Buffer req_;
  int sentlen_;

  Buffer rep_;

  int fd_;
  int flags_;
  time_t last_active_;

  NetworkStore *store_;
};

NetworkStore::Agent *NetworkStore::Agent::Create(const int fd, NetworkStore *store) {
  Agent *s = new Agent();
  s->eventl_ = store->eventl_;

  anetNonBlock(NULL, fd);
  anetEnableTcpNoDelay(NULL, fd);
  s->fd_ = fd;
  s->sentlen_ = 0;
  s->store_ = store;
  s->flags_ = 0;

  if (aeCreateFileEvent(s->eventl_, fd, AE_READABLE, ReadReply, s) == AE_ERR) {
    LogError("%s", strerror(errno));
    delete s;
    return NULL;
  }

  return s;
}

NetworkStore::Agent::~Agent() {
  LogDebug("close remote kids server");
  aeDeleteFileEvent(eventl_, fd_, AE_READABLE);
  aeDeleteFileEvent(eventl_, fd_, AE_WRITABLE);
  if (fd_ > 0) close(fd_);
}

bool NetworkStore::Agent::SetWriteEvent() {
  if (fd_ <= 0) return false;
  if (req_.size() != 0) return true;
  if (aeCreateFileEvent(eventl_, fd_, AE_WRITABLE, SendRequest, this) == AE_ERR) {
    LogError("Set write event failed");
    return false;
  }
  return true;
}

void NetworkStore::Agent::Log(const Message *msg) {
  if (!SetWriteEvent()) {
    LogError("set write event failed");
    return;
  }

  LogDebug("log to remote server, req buflen: %d, data: %s:%s", req_.size(), msg->topic, msg->content);

  req_.append_printf("*3\r\n$3\r\nLOG\r\n$%d\r\n", sdslen(msg->topic));
  req_.append(msg->topic, sdslen(msg->topic));
  req_.append("\r\n", 2);
  req_.append_printf("$%d\r\n", sdslen(msg->content));
  req_.append(msg->content, sdslen(msg->content));
  req_.append("\r\n", 2);
}

bool NetworkStore::Agent::ParseReplyBuffer() {
  char *newline = NULL;
  newline = strchr(rep_.data(), '\r');
  if (newline == NULL) return false;
  // Buffer should also contain '\n'
  if (newline-(rep_.data()) > (rep_.size()-2)) return false;
  rep_.remove_front(newline-(rep_.data()) + 2);
  return true;
}

bool NetworkStore::Agent::ProcessReplyBuffer() {
  while(rep_.size() > 0) {

    if (!ParseReplyBuffer()) break;

    if (ProcessResult()) {
      Reset();
    }
  }

  if (rep_.size() == 0) rep_.clear();

  return true;
}

void NetworkStore::Agent::Reset() {
  LogDebug("reset server");
}

std::string NetworkStore::Agent::ToString() {
  char ip[32], flags[16], events[3], *p;
  int port;
  int emask;

  if (anetPeerToString(fd_, ip, sizeof(ip), &port) == -1) {
    ip[0] = '?';
    ip[1] = '\0';
    port = 0;
  }
  p = flags;
  if (flags_ & CLIENT_CLOSE_AFTER_REPLY) *p++ = 'c';
  if (p == flags) *p++ = 'N';
  *p++ = '\0';

  emask = (fd_ == -1 ? 0 : aeGetFileEvents(eventl_, fd_));
  p = events;
  if (emask & AE_READABLE) *p++ = 'r';
  if (emask & AE_WRITABLE) *p++ = 'w';
  *p = '\0';
  Buffer buf;
  buf.append_printf("addr=%s:%d fd=%d idle=%ld flags=%s qbuf=%d omem=%d events=%s",
                    ip, port, fd_,
                    (long)(unixtime - last_active_),
                    flags,
                    req_.size(),
                    rep_.size(),
                    events);
  return std::string(buf.data());
}

bool NetworkStore::Agent::ProcessResult() {
  // kids should never fail when network is well
  store_->OnMsgSent();
  return true;
}

NetworkStore::NetworkStore(StoreConfig *conf, aeEventLoop *el) :Store(conf), eventl_(el) {
  agent_ = NULL;
  reconnect_interval_ = 5;
  last_reconnect_ = 0;
  if (!conf->socket.empty()) {
    socket_ = conf->socket;
  } else if (!conf->host.empty() && !conf->port.empty()) {
    host_ = conf->host;
    port_ = atoi(conf->port.c_str());
  } else {
    LogError(ERR_BAD_ADDR);
  }
}

NetworkStore::~NetworkStore() {
  Close();
}

bool NetworkStore::Open() {
  if (IsOpen()) Close();
  last_reconnect_ = unixtime;
  char error[1024] = {0};

  int fd;

  if (!host_.empty()) {
    LogInfo("connect NetworkStore %s:%d", host_.c_str(), port_);
    fd = anetTcpConnect(error, const_cast<char*>(host_.c_str()), port_);
  } else {
    LogInfo("connect NetworkStore %s", socket_.c_str());
    fd = anetUnixConnect(error, const_cast<char*>(socket_.c_str()));
  }
  if (fd == -1) {
    LogWarning("Unable to connect to NetworkStore: %s", error);
    return false;
  }

  agent_ = Agent::Create(fd, this);
  if (agent_ == NULL) {
    LogError("open NetworkStore failed");
  }

  state_ = Store::Free;

  LogDebug("opened NetworkStore");
  return IsOpen();
}

bool NetworkStore::IsOpen() {
  return agent_ != NULL;
}

void NetworkStore::Close() {
  if (!IsOpen()) return;
  LogDebug("closing NetworkStore...");
  delete agent_;
  agent_ = NULL;
  state_ = Store::Free;
}

bool NetworkStore::DoAddMessage(const Message *msg) {
  if (!IsOpen()) return false;
  LogDebug("add message %p", msg);
  agent_->Log(msg);
  state_ = Store::Sending;
  return true;
}

void NetworkStore::OnMsgSent() {
  state_ = Store::Free;
}

void NetworkStore::OnDisconnect() {
  Close();
}

void NetworkStore::Cron() {
  if (IsOpen()) return;
  time_t t = unixtime;
  if (t - last_reconnect_ >= reconnect_interval_) {
    LogDebug("reconnect %d %d %d", t, last_reconnect_, reconnect_interval_);
    Open();
  }
}


void SendRequest(aeEventLoop *el, int fd, void *privdata, int mask) {
  NetworkStore::Agent *s = static_cast<NetworkStore::Agent*>(privdata);
  int nwritten = 0, totwritten = 0;

  while (s->sentlen_ < s->req_.size()) {
    if (s->sentlen_ < s->req_.size()) {
      nwritten = write(fd, s->req_.data() + s->sentlen_, s->req_.size() - s->sentlen_);
      if (nwritten <= 0) break;

      s->sentlen_ += nwritten;
      totwritten += nwritten;

      // If the buffer was sent, set sentlen_ to zero to continue with
      // the remainder of the reply.
      if (s->sentlen_ == s->req_.size()) {
        s->sentlen_ = 0;
        s->req_.clear();
        LogDebug("clearred request buffer %d", s->req_.size());
      }
    }
    // Note that we avoid to send more thank KIDS_MAX_WRITE_PER_EVENT
    // bytes, in a single threaded server it's a good idea to serve
    // other clients as well, even if a very large request comes from
    // super fast link that is always able to accept data
    if (totwritten > KIDS_MAX_WRITE_PER_EVENT) break;
  }

  if (nwritten == -1) {
    if (errno != EAGAIN) {
      LogError("error writing to client: %s", strerror(errno));
      s->store_->OnDisconnect();
      return;
    }
  }

  if (s->sentlen_ == s->req_.size()) {
    aeDeleteFileEvent(el, fd, AE_WRITABLE);
  }
}

void ReadReply(aeEventLoop *el, int fd, void *privdata, int mask) {
  NetworkStore::Agent *s = static_cast<NetworkStore::Agent*>(privdata);
  char buf[KIDS_IOBUF_LEN];
  int nread;

  nread = read(fd, buf, KIDS_IOBUF_LEN);
  if (nread == -1) {
    if (errno == EAGAIN) {
      nread = 0;
    } else {
      LogError("Reading from remote kids: %s", strerror(errno));
      goto ERROR;
    }
  } else if (nread == 0) {
    LogInfo("Remote kids closed connection");
    goto ERROR;
  }

  if (nread) {
    s->rep_.append(buf, nread);
    s->last_active_ = unixtime;
  } else {
    return;
  }

  if (s->req_.size() > KIDS_MAX_QUERYBUF_LEN) {
    LogWarning("Closing client that reached max query buffer length: %s", s->ToString().c_str());
    goto ERROR;
  }

  s->ProcessReplyBuffer();
  return;

ERROR:
  s->store_->OnDisconnect();
}
