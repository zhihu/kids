#ifndef __KIDS_COMMON_H_
#define __KIDS_COMMON_H_

#include <cstdint>
#include <cstring>
#include "sds.h"

typedef struct Statistic {
  Statistic() {
    start_time         = 0;
    msg_in             = 0;
    msg_in_traffic     = 0;
    msg_in_ps          = 0;
    msg_in_traffic_ps  = 0;
    msg_out            = 0;
    msg_out_traffic    = 0;
    msg_out_ps         = 0;
    msg_out_traffic_ps = 0;
    msg_in_queue       = 0;
    queue_mem_size     = 0;
    msg_store          = 0;
    msg_store_size     = 0;
    msg_buffer         = 0;
    msg_buffer_size    = 0;
    msg_drop           = 0;
    clients            = 0;
    topics             = 0;
    patterns           = 0;
  }
  void Add(const Statistic &stat) {
    msg_in          += stat.msg_in;
    msg_in_traffic  += stat.msg_in_traffic;
    msg_out         += stat.msg_out;
    msg_out_traffic += stat.msg_out_traffic;
    msg_in_queue    += stat.msg_in_queue;
    queue_mem_size  += stat.queue_mem_size;
    msg_store       += stat.msg_store;
    msg_store_size  += stat.msg_store_size;
    msg_buffer      += stat.msg_buffer;
    msg_buffer_size += stat.msg_buffer_size;
    msg_drop        += stat.msg_drop;
    clients         += stat.clients;
    topics          += stat.topics;
    patterns        += stat.patterns;
  }

  time_t start_time;
  uint64_t msg_in;
  uint64_t msg_in_traffic;
  uint64_t msg_in_ps;
  uint64_t msg_in_traffic_ps;
  uint64_t msg_out;
  uint64_t msg_out_traffic;
  uint64_t msg_out_ps;
  uint64_t msg_out_traffic_ps;
  uint64_t msg_in_queue;
  uint64_t queue_mem_size;
  uint64_t msg_store;
  uint64_t msg_store_size;
  uint64_t msg_buffer;
  uint64_t msg_buffer_size;
  uint64_t msg_drop;
  uint64_t clients;
  uint64_t topics;
  uint64_t patterns;
} Statistic;

struct SdsHasher {
  size_t operator()(const sds &buf) const {
    size_t hash = 5381;
    for (size_t i = 0; i < sdslen(buf); ++i) {
      hash = ((hash << 5) + hash) + buf[i];
    }
    return hash;
  }
};

struct SdsEqual {
  bool operator()(const sds &a, const sds &b) const {
    if (sdslen(a) != sdslen(b)) return false;
    return !memcmp(a, b, sdslen(a));
  }
};

#endif  // __KIDS_COMMON_H_
