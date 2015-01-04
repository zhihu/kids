#include "client.h"
#include "kids.h"

void ReadRequest(aeEventLoop *el, int fd, void *privdata, int mask) {
  Client *c = static_cast<Client*>(privdata);

  char buf[KIDS_IOBUF_LEN];
  int nread;

  nread = read(fd, buf, KIDS_IOBUF_LEN);
  if (nread == -1) {
    if (errno != EAGAIN) {
      LogError("Error reading from client: %s", strerror(errno));
      c->worker_->FreeClient(c);
    }
    return;
  }

  if (nread == 0) {
    LogInfo("Client closed connection");
    c->worker_->FreeClient(c);
    return;
  }

  if (!c->req_.append(buf, nread)) {
    c->worker_->FreeClient(c);
    return;
  }

  if (c->req_.size() > KIDS_MAX_QUERYBUF_LEN) {
    LogWarning("Closing client that reached max query buffer length : %s", c->ToString().c_str());
    c->worker_->FreeClient(c);
    return;
  }

  c->last_active_ = unixtime;
  c->ProcessRequestBuffer();
}

void SendReply(aeEventLoop *el, int fd, void *privdata, int mask) {
  Client *c = static_cast<Client*>(privdata);

  LogDebug("send reply %s", c->rep_.data());
  int nwritten = 0, totwritten = 0;

  while (c->sentlen_ < c->rep_.size()) {
    nwritten = write(fd, c->rep_.data() + c->sentlen_, c->rep_.size() - c->sentlen_);
    if (nwritten <= 0) break;

    c->sentlen_ += nwritten;
    totwritten += nwritten;

    if (c->sentlen_ == c->rep_.size()) {
      /* the buffer has been sent */
      c->sentlen_ = 0;
      if (c->rep_.size() > CLIENT_REP_BUF_BYTES) {
        c->rep_.resize(CLIENT_REP_BUF_BYTES);
      }
      c->rep_.set_size(0);
      break;
    }
    // Note that we avoid to send more thank REDIS_MAX_WRITE_PER_EVENT
    // bytes, in a single threaded server it's a good idea to serve
    // other clients as well, even if a very large req_uest comes from
    // super fast link that is always able to accept data (in real world
    // scenario think about 'KEYS *' against the loopback interfae)
    if (totwritten > KIDS_MAX_WRITE_PER_EVENT) break;
  }

  if (nwritten == -1) {
    if (errno != EAGAIN) {
      LogError("Error writing to client: %s", strerror(errno));
      c->worker_->FreeClient(c);
      return;
    }
  }
  if (totwritten > 0) c->last_active_ = unixtime;
  if (c->sentlen_ >= c->rep_.size()) {
    aeDeleteFileEvent(el, fd, AE_WRITABLE);
    // Close connection after entire reply has been sent.
    if (c->flags_ & CLIENT_CLOSE_AFTER_REPLY) {
      c->worker_->FreeClient(c);
    }
  }
}

Client *Client::Create(const int fd, Worker *t) {
  Client *c = new Client(fd, t);

  if (aeCreateFileEvent(t->eventl_, c->fd_, AE_READABLE, ReadRequest, c) == AE_ERR) {
    LogError("%s", strerror(errno));
    delete c;
    return NULL;
  }
  return c;
}

Client::Client(const int fd, Worker *t)
    : fd_(fd), rep_(CLIENT_REP_BUF_BYTES), last_cmd_("NULL"), worker_(t)
{
  anetNonBlock(NULL, fd);
  anetEnableTcpNoDelay(NULL, fd);

  sentlen_ = 0;
  argc_ = 0;
  cur_arglen_ = -1;

  flags_ = 0x0;
  last_active_ = 0;
  last_soft_limit_ = 0;
}

Client::~Client() {
  LogDebug("close Client %d", fd_);

  aeDeleteFileEvent(worker_->eventl_, fd_, AE_READABLE);
  aeDeleteFileEvent(worker_->eventl_, fd_, AE_WRITABLE);

  if (kids->config_.max_clients != 0) {
    char ip[32];
    anetPeerToString(fd_, ip, sizeof(ip), nullptr);
    kids->RemoveClient(std::string(ip));
  }

  close(fd_);

  UnsubscribeAllTopics(false);
  UnsubscribeAllPatterns(false);
}

void Client::ReceiveMessage(const sds &topic, const sds &content) {
  Reply(REP_MBULK3, REP_MBULK3_SIZE);
  Reply(REP_MESSAGEBULK, REP_MESSAGEBULK_SIZE);
  ReplyBulk(topic, sdslen(topic));
  ReplyBulk(content, sdslen(content));
}

void Client::ReceivePatternMessage(const sds &topic, const sds &content, const sds &pattern) {
  Reply(REP_MBULK4, REP_MBULK4_SIZE);
  Reply(REP_PMESSAGEBULK, REP_PMESSAGEBULK_SIZE);
  ReplyBulk(pattern, sdslen(pattern));
  ReplyBulk(topic, sdslen(topic));
  ReplyBulk(content, sdslen(content));
}

std::string Client::ToString() {
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
  if (flags_ & CLIENT_CLOSE_ASAP) *p++ = 'A';
  if (p == flags) *p++ = 'N';
  *p++ = '\0';

  emask = fd_ == -1 ? 0 : aeGetFileEvents(worker_->eventl_, fd_);
  p = events;
  if (emask & AE_READABLE) *p++ = 'r';
  if (emask & AE_WRITABLE) *p++ = 'w';
  *p = '\0';
  Buffer buf;
  buf.append_printf("addr=%s:%d fd=%d idle=%ld flags=%s sub=%d psub=%d qbuf=%d omem=%d events=%s cmd=%s",
                    ip, port, fd_,
                    (long)(unixtime - last_active_),
                    flags,
                    sub_topics_.size(),
                    sub_patterns_.size(),
                    req_.size(),
                    rep_.size(),
                    events,
                    last_cmd_.c_str());
  return std::string(buf.data());
}

void Client::ProcessRequestBuffer() {
  while (req_.size() > 0) {
    if (flags_ & CLIENT_CLOSE_AFTER_REPLY) break;
    if (!ParseRequestBuffer()) break;

    if (argv_.size() == 0 || ProcessCommand()) {
      ResetArgumentVector();
    }
  }

  if (req_.size() == 0) req_.clear();
}

// parse redis protocol
bool Client::ParseRequestBuffer() {
  char *newline = NULL;
  int pos = 0, ok;
  long long ll;

  if (argc_ == 0) {
    // Multi bulk length cannot be read without a '\r\n'
    newline = strchr(req_.data(), '\r');
    if (newline == NULL) return false;
    // Buffer should also contain '\n'
    if (req_.size() < newline - req_.data() + 2 ) return false;

    // We know for sure there is a whole line since newline != NULL,
    // so go ahead and find out the multi bulk length.
    // assert(req_->data()[0] == '*');
    ok = string2ll(req_.data(1), newline - (req_.data(1)), &ll);
    if (!ok || ll > 1024*1024) {
      LogDebug("Protocol error: invalid multibulk length");
      ProcessProtocolError(pos);
      return false;
    }

    pos = (newline - req_.data()) + 2;
    if (ll <= 0) {
      req_.remove_front((newline - req_.data()) + 2);
      return true;
    }

    argc_ = ll;

    // Setup argv_ array on client structure
    if (argv_.size() > 0) argv_.clear();
  }

  // assert(c->multibulklen > 0);
  while (argv_.size() < argc_) {
    // Read arg length if unknown
    if (cur_arglen_ == -1) {
      newline = strchr(req_.data(pos), '\r');
      if (newline == NULL) break;
      // Buffer should also contain '\n'
      if (req_.size() < newline - req_.data() + 2) break;

      if (req_[pos] != '$') {
        LogDebug("Protocol error: expected '$', got '%c'", req_[pos]);
        ReplyErrorFormat("Protocol error: expected '$', got '%c'", req_[pos]);
        ProcessProtocolError(pos);
        return false;
      }

      ok = string2ll(req_.data(pos+1), newline - req_.data(pos+1), &ll);
      if (!ok || ll < 0 || ll > 512*1024*1024) {
        LogDebug("Protocol error: invalid bulk length");
        ReplyErrorFormat("Protocol error: invalid bulk length");
        ProcessProtocolError(pos);
        return false;
      }

      pos += newline - req_.data(pos) + 2;
      cur_arglen_ = ll;
    }

    // Read argument
    if (req_.size() - pos < cur_arglen_ + 2) {
      // Not enough data (+2 == trailing '\r\n')
      LogDebug("not enough data : [%s]", req_.data(pos));
      break;
    } else {
      sds t = sdsnewlen(req_.data(pos), cur_arglen_);
      argv_.push_back(t);
      pos += cur_arglen_ + 2;
      cur_arglen_ = -1;
    }
  }

  // Trim to pos
  req_.remove_front(pos);

  // We're done when c->multibulk == 0
  if (argc_ == argv_.size()) {
    return true;
  }

  return false;
}

void Client::Reply(const char *data, uint32_t size) {
  if (flags_ & CLIENT_CLOSE_AFTER_REPLY) return;
  if (!SetWriteEvent()) return;

  rep_.append(data, size);
  AsyncCloseOnOutputBufferLimitsReached();
}

void Client::ReplyBulk(const char *data, uint32_t size) {
  if (flags_ & CLIENT_CLOSE_AFTER_REPLY) return;
  if (!SetWriteEvent()) return;

  rep_.append_printf("$%lu\r\n", size);
  rep_.append(data, size);
  rep_.append("\r\n", 2);
  AsyncCloseOnOutputBufferLimitsReached();
}

void Client::ReplyBulkFormat(const char *fmt, ...) {
  if (flags_ & CLIENT_CLOSE_AFTER_REPLY) return;
  if (!SetWriteEvent()) return;

  Buffer buf;
  va_list ap;

  va_start(ap, fmt);
  buf.append_vaprintf(fmt, ap);
  va_end(ap);

  rep_.append_printf("$%lu\r\n", buf.size());

  rep_.append(buf.data(), buf.size());
  rep_.append("\r\n", 2);
  AsyncCloseOnOutputBufferLimitsReached();
}

void Client::ReplyStatusFormat(const char *fmt, ...) {
  if (flags_ & CLIENT_CLOSE_AFTER_REPLY) return;
  if (!SetWriteEvent()) return;

  va_list ap;
  rep_.append("+", 1);
  va_start(ap, fmt);
  rep_.append_vaprintf(fmt, ap);
  va_end(ap);
  rep_.append("\r\n", 2);
  AsyncCloseOnOutputBufferLimitsReached();
}

void Client::ReplyErrorFormat(const char *fmt, ...) {
  if (flags_ & CLIENT_CLOSE_AFTER_REPLY) return;
  if (!SetWriteEvent()) return;

  va_list ap;
  rep_.append("-ERR ", 5);
  va_start(ap, fmt);
  rep_.append_vaprintf(fmt, ap);
  va_end(ap);
  rep_.append("\r\n", 2);
  AsyncCloseOnOutputBufferLimitsReached();
}

void Client::ReplyLongLong(long long ll) {
  if (ll == 0) {
    Reply(REP_CZERO, REP_CZERO_SIZE);
  } else if (ll == 1) {
    Reply(REP_CONE, REP_CONE_SIZE);
  } else {
    char buf[128];
    int len;
    buf[0] = ':';
    len = ll2string(buf + 1, sizeof(buf) - 1, ll);
    buf[len+1] = '\r';
    buf[len+2] = '\n';
    Reply(buf, len + 3);
  }
}

bool Client::SetWriteEvent() {
  if (fd_ <= 0) return false;
  if (sentlen_ != 0) return false;

  if (aeCreateFileEvent(worker_->eventl_, fd_, AE_WRITABLE, SendReply, this) == AE_ERR) {
    LogError("Set write event failed");
    return false;
  }
  return true;
}

bool Client::CheckOutputBufferLimits() {
  int type = NLIMIT_NORMAL;
  if (sub_topics_.size() > 0 || sub_patterns_.size() > 0) type = NLIMIT_PUBSUB;
  int soft = 0, hard = 0;

  if (kids->config_.nlimit[type].hard_limit_bytes &&
      rep_.size() >= kids->config_.nlimit[type].hard_limit_bytes)
    hard = 1;
  if (kids->config_.nlimit[type].soft_limit_bytes &&
      rep_.size() >= kids->config_.nlimit[type].soft_limit_bytes)
    soft = 1;

  // We need to check if the soft limit is reached continuously for the
  // specified amount of seconds.
  if (soft) {
    if (last_soft_limit_ == 0) {
      last_soft_limit_ = unixtime;
      soft = 0;  // First time we see the soft limit reached
    } else {
      time_t elapsed = unixtime - last_soft_limit_;
      if (elapsed <= kids->config_.nlimit[type].soft_limit_seconds) {
        // The client still did not reached the max number of
        // seconds for the soft limit to be considered reached.
        soft = 0;
      }
    }
  } else {
    last_soft_limit_ = 0;
  }

  return soft || hard;
}

void Client::AsyncCloseOnOutputBufferLimitsReached() {
  if (flags_ & CLIENT_CLOSE_ASAP) return;
  if (CheckOutputBufferLimits()) {
    flags_ |= CLIENT_CLOSE_ASAP;
    worker_->FreeClientAsync(this);
    LogInfo("client: %s scheduled to be closed ASAP for overcoming of output buffer limits.", ToString().c_str());
  }
}

void Client::ResetArgumentVector() {
  LogDebug("reset client argument vector");
  for (auto i : argv_) {
    sdsfree(i);
  }
  argv_.clear();
  cur_arglen_ = -1;
  argc_ = 0;
}

bool Client::ProcessCommand() {
  LogDebug("process command '%s' argc_: %d", argv_[0], argv_.size());
  LogDebug("client: %s", ToString().c_str());

  const char* command = argv_[0];
  if (!strcasecmp(command, "quit")) {
    ProcessQuit();
    return false;
  } else if (!strcasecmp(command, "ping")) {
    ProcessPing();
  } else if (!strcasecmp(command, "info")) {
    ProcessInfo();
  } else if (!strcasecmp(command, "publish") || !strcasecmp(command, "log")) {
    ProcessLog();
  } else if (!strcasecmp(command, "subscribe")) {
    ProcessSubscribe();
  } else if (!strcasecmp(command, "psubscribe")) {
    ProcessPSubscribe();
  } else if (!strcasecmp(command, "unsubscribe")) {
    ProcessUnSubscribe(true);
  } else if (!strcasecmp(command, "punsubscribe")) {
    ProcessPUnSubscribe(true);
  } else if (!strcasecmp(command, "shutdown")) {
    ProcessShutdown();
  } else if (!strcasecmp(command, "select")) {
    Reply(REP_OK, REP_OK_SIZE);
  } else if (!strcasecmp(command, "hello")) {
    ReplyStatusFormat("World!");
  } else {
    ProcessOther();
  }

  last_cmd_ = argv_[0];

  return true;
}

void Client::ProcessQuit() {
  Reply(REP_OK, REP_OK_SIZE);
  flags_ |= CLIENT_CLOSE_AFTER_REPLY;
}

void Client::ProcessLog() {
  if (argv_.size() != 3) {
    ReplyErrorFormat("invalid argments of log");
  } else {
    LogDebug("processlog(size:%d): %s:%s", sdslen(argv_[2]), argv_[1], argv_[2]);
    worker_->stat_.msg_in++;
    worker_->stat_.msg_in_traffic += (sdslen(argv_[1]) + sdslen(argv_[2]));

    if (kids->PutMessage(argv_[1], argv_[2], worker_->worker_id_)) {
      Reply(REP_CONE, REP_CONE_SIZE);
    } else {
      ReplyErrorFormat("Some thing bad happens!");
    }
    argv_[1] = NULL;
    argv_[2] = NULL;
  }
}

void Client::ProcessOther() {
  static const char *redis_cmd[] = {
    "get", "set", "setnx", "setex", "psetex", "append", "strlen", "del", "exists", "setbit", "getbit", "setrange", "getrange",
    "substr", "incr", "decr", "mget", "rpush", "lpush", "rpushx", "lpushx", "linsert", "rpop", "lpop", "brpop", "brpoplpush",
    "blpop", "llen", "lindex", "lset", "lrange", "ltrim", "lrem", "rpoplpush", "sadd", "srem", "smove", "sismember", "scard",
    "spop", "srandmember", "sinter", "sinterstore", "sunion", "sunionstore", "sdiff", "sdiffstore", "smembers", "zadd", "zincrby",
    "zrem", "zremrangebyscore", "zremrangebyrank", "zunionstore", "zinterstore", "zrange", "zrangebyscore", "zrevrangebyscore", "zcount",
    "zrevrange", "zcard", "zscore", "zrank", "zrevrank", "hset", "hsetnx", "hget", "hmset", "hmget", "hincrby", "hincrbyfloat",
    "hdel", "hlen", "hkeys", "hvals", "hgetall", "hexists", "incrby", "decrby", "incrbyfloat", "getset", "mset", "msetnx",
    "randomkey", "select", "move", "rename", "renamenx", "expire", "expireat", "pexpire", "pexpireat", "keys", "dbsize", "auth",
    "ping", "echo", "save", "bgsave", "bgrewriteaof", "shutdown", "lastsave", "type", "multi", "exec", "discard", "sync",
    "flushdb", "flushall", "sort", "info", "monitor", "ttl", "pttl", "persist", "slaveof", "debug", "config", "subscribe",
    "unsubscribe", "psubscribe", "punsubscribe", "publish", "watch", "unwatch", "cluster", "restore", "migrate", "asking", "dump",
    "object", "client", "eval", "evalsha", "slowlog", "script", NULL
  };

  LogInfo("Unknown command %s", argv_[0]);

  for (int i = 0; redis_cmd[i] != NULL; i++) {
    if (!strcasecmp(redis_cmd[i], argv_[0])) {
      ReplyErrorFormat("Sorry this is KIDS but not Redis, '%s' is not supported", argv_[0]);
      return;
    }
  }

  ReplyErrorFormat("Unknown command '%s'", argv_[0]);
}

void Client::ProcessShutdown() {
  LogInfo("User shutdown");
  kill(getpid(), SIGTERM);
}

void Client::ProcessPing() {
  Reply(REP_PONG, REP_PONG_SIZE);
}

void Client::ProcessInfo() {
  Statistic statistic = kids->stat_;

  time_t uptime = unixtime - statistic.start_time;
  struct rusage self_ru, c_ru;
  const char *section = "";
  char hostname[100];
  int nsection = 0;

  if (argv_.size() > 1) {
    section = argv_[1];
  }

  getrusage(RUSAGE_SELF, &self_ru);
  getrusage(RUSAGE_CHILDREN, &c_ru);
  gethostname(hostname, 100);

  Buffer buf;

  if (argv_.size() == 1 || !strcasecmp(section, "server")) {
    if (nsection++) buf.append_printf("\r\n");
    buf.append_printf("# Server\r\n"
                      "kids_version:" VERSION " %sbit\r\n"
                      "server_hostname:%s\r\n"
                      "multiplexing_api:%s\r\n"
                      "process_id:%d\r\n"
                      "tcp_port:%d\r\n"
                      "unix_socket:%s\r\n"
                      "uptime_in_seconds:%d\r\n"
                      "uptime_in_days:%d\r\n"
                      "connected_clients:%d\r\n",
                      (sizeof(long) == 8) ? "64" : "32",
                      hostname,
                      aeGetApiName(),
                      getpid(),
                      kids->config_.listen_port,
                      kids->config_.listen_socket.c_str(),
                      uptime,
                      uptime/(3600*24),
                      statistic.clients);
  }

  if (argv_.size() == 1 || !strcasecmp(section, "message") || !strcasecmp(section, "msg")) {
    if (nsection++) buf.append_printf("\r\n");
    char msg_in_traffic[64];     bytesToHuman(msg_in_traffic, statistic.msg_in_traffic);
    char msg_in_traffic_ps[64];  bytesToHuman(msg_in_traffic_ps, statistic.msg_in_traffic_ps);
    char msg_out_traffic[64];    bytesToHuman(msg_out_traffic, statistic.msg_out_traffic);
    char msg_out_traffic_ps[64]; bytesToHuman(msg_out_traffic_ps, statistic.msg_out_traffic_ps);
    char queue_mem_size[64];     bytesToHuman(queue_mem_size, statistic.queue_mem_size);
    char msg_store_size[64];     bytesToHuman(msg_store_size, statistic.msg_store_size);
    char msg_buffer_size[64];    bytesToHuman(msg_buffer_size, statistic.msg_buffer_size);

    buf.append_printf("# Messages\r\n"
                      "message_in:%llu\r\n"
                      "message_in_traffic:%llu\r\n"
                      "message_in_traffic_human:%s\r\n"
                      "message_in_per_second:%llu\r\n"
                      "message_in_traffic_per_second:%llu\r\n"
                      "message_in_traffic_per_second_human:%s\r\n"
                      "message_out:%llu\r\n"
                      "message_out_traffic:%llu\r\n"
                      "message_out_traffic_human:%s\r\n"
                      "message_out_per_second:%llu\r\n"
                      "message_out_traffic_per_second:%llu\r\n"
                      "message_out_traffic_per_second_human:%s\r\n"
                      "message_in_queue:%llu\r\n"
                      "queue_mem_size:%llu\r\n"
                      "queue_mem_size_human:%s\r\n"
                      "message_store:%llu\r\n"
                      "message_store_size:%llu\r\n"
                      "message_store_size_human:%s\r\n"
                      "message_buffer:%llu\r\n"
                      "message_buffer_size:%llu\r\n"
                      "message_buffer_size_human:%s\r\n"
                      "message_drop:%llu\r\n"
                      "pubsub_topics:%zu\r\n"
                      "pubsub_patterns:%zu\r\n",
                      statistic.msg_in,
                      statistic.msg_in_traffic,
                      msg_in_traffic,
                      statistic.msg_in_ps,
                      statistic.msg_in_traffic_ps,
                      msg_in_traffic_ps,
                      statistic.msg_out,
                      statistic.msg_out_traffic,
                      msg_out_traffic,
                      statistic.msg_out_ps,
                      statistic.msg_out_traffic_ps,
                      msg_out_traffic_ps,
                      statistic.msg_in_queue,
                      statistic.queue_mem_size,
                      queue_mem_size,
                      statistic.msg_store,
                      statistic.msg_store_size,
                      msg_store_size,
                      statistic.msg_buffer,
                      statistic.msg_buffer_size,
                      msg_buffer_size,
                      statistic.msg_drop,
                      statistic.topics,
                      statistic.patterns);
  }

  if (argv_.size() == 1 || !strcasecmp(section, "resource") || !strcasecmp(section, "res")) {
    char hmem[64];
    bytesToHuman(hmem, zmalloc_get_rss());
    if (nsection++) buf.append_printf("\r\n");
    buf.append_printf("# Resource\r\n"
                      "used_memory_rss:%zu\r\n"
                      "used_memory_rss_human:%s\r\n"
                      "used_cpu_sys:%.2f\r\n"
                      "used_cpu_user:%.2f\r\n",
                      zmalloc_get_rss()/1024,
                      hmem,
                      (float)self_ru.ru_stime.tv_sec+(float)self_ru.ru_stime.tv_usec/1000000,
                      (float)self_ru.ru_utime.tv_sec+(float)self_ru.ru_utime.tv_usec/1000000);
  }

  ReplyBulk(buf.data(), buf.size());
}

void Client::ProcessProtocolError(int pos) {
  LogInfo("Protocol Error from client: %s", ToString().c_str());
  flags_ |= CLIENT_CLOSE_AFTER_REPLY;
}
