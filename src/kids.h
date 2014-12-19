#ifndef __KIDS_KIDS_H_
#define __KIDS_KIDS_H_

#include <errno.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

#include <climits>
#include <cstdint>
#include <cstdarg>
#include <cstring>

#include <algorithm>
#include <string>
#include <list>
#include <vector>

#include "ae/libae.h"
#include "buffer.h"
#include "config.h"
#include "conf.h"
#include "client.h"
#include "constants.h"
#include "logger.h"
#include "msgqueue.h"
#include "master.h"
#include "storer.h"
#include "store/store.h"
#include "sds.h"
#include "util.h"
#include "worker.h"

#define CLIENT_CLOSE_AFTER_REPLY 0x01
#define CLIENT_CLOSE_ASAP 0x02
#define CLIENT_TCP 0x04
#define CLIENT_UNIX_SOCKET 0x08
#define CLIENT_REP_BUF_BYTES (1024*16)

#define KIDS_IOBUF_LEN (1024*16)
#define KIDS_MAX_QUERYBUF_LEN (1024*1024*1024)

#define KIDS_MAX_WRITE_PER_EVENT (1024*64)

class NetworkStore;

struct Pattern {
  Pattern(const sds p, Client* c) : pattern(p), client(c) {}
  sds pattern;
  Client *client;
};

void AcceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask);
void AcceptUnixHandler(aeEventLoop *el, int fd, void *privdata, int mask);
void ReadRequest(aeEventLoop *el, int fd, void *privdata, int mask);
void SendReply(aeEventLoop *el, int fd, void *privdata, int mask);

extern Master *kids;
extern __thread time_t unixtime;
#endif  // __KIDS_KIDS_H_
