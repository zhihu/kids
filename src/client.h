#ifndef __KIDS_CLIENT_H_
#define __KIDS_CLIENT_H_

#include <vector>
#include <string>
#include <unordered_set>

#include "buffer.h"
#include "common.h"

struct aeEventLoop;
class Worker;
typedef char* sds;

class Client {
  friend void ReadRequest(aeEventLoop *el, int fd, void *privdata, int mask);
  friend void SendReply(aeEventLoop *el, int fd, void *privdata, int mask);

 public:
  static Client *Create(const int fd, Worker *t);
  ~Client();

  void ReceiveMessage(const sds &topic, const sds &content);
  void ReceivePatternMessage(const sds &topic, const sds &content, const sds &pattern);

  std::string ToString();

 private:
  typedef std::unordered_set<sds, SdsHasher, SdsEqual> BufferSet;

  Client(const int fd, Worker *t);

  void ProcessRequestBuffer();
  bool ParseRequestBuffer();

  void Reply(const char *data, uint32_t size);
  void ReplyBulk(const char *data, uint32_t size);
  void ReplyBulkFormat(const char *fmt, ...);
  void ReplyStatusFormat(const char *fmt, ...);
  void ReplyErrorFormat(const char *fmt, ...);
  void ReplyLongLong(long long ll);

  bool SetWriteEvent();
  bool CheckOutputBufferLimits();
  void AsyncCloseOnOutputBufferLimitsReached();
  void ResetArgumentVector();

  // command handlers
  bool ProcessCommand();
  void ProcessPing();
  void ProcessInfo();
  void ProcessQuit();
  void ProcessLog();
  void ProcessSubscribe();
  void ProcessPSubscribe();
  void ProcessUnSubscribe(bool notify);
  void ProcessPUnSubscribe(bool notify);
  void ProcessShutdown();
  void ProcessOther();
  void ProcessProtocolError(const int pos);
  void UnsubscribeTopic(const sds &topic, bool notify);
  void UnsubscribePattern(const sds &pattern, bool notify);
  void UnsubscribeAllTopics(bool notify);
  void UnsubscribeAllPatterns(bool notify);

  const int fd_;

  Buffer req_;
  Buffer rep_;
  int sentlen_;

  int argc_;
  int cur_arglen_;
  std::vector<sds> argv_;

  int flags_;
  time_t last_active_;
  time_t last_soft_limit_;
  std::string last_cmd_;

  BufferSet sub_topics_;
  BufferSet sub_patterns_;

  Worker *worker_;
};

#endif  // __KIDS_CLIENT_H_
