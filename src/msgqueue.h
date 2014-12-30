#ifndef __KIDS_MSGQUEUE_H_
#define __KIDS_MSGQUEUE_H_

#include <pthread.h>
#include <string>

#include "sds.h"

/* make sure topic and content is only owned by this message
 * namely, do not allocate message passing topic or content owned by other messages
 */
struct Message {
  Message():topic(NULL), content(NULL) {}
  Message(const sds t, const sds c) : topic(t), content(c) {}
  ~Message() {
    sdsfree(topic);
    sdsfree(content);
  }
  const sds topic;
  const sds content;
};

typedef struct MessageQueueItem MQItem;
struct MessageQueueItem {
  MessageQueueItem(const sds t, const sds c, MQItem *n) {
    value = new Message(t, c);
    next = n;
  }
  ~MessageQueueItem() {
    delete value;
  }
  Message *value;
  MQItem *next;
};

typedef struct MessageQueue MQ;
struct MessageQueue {
  MQItem *head;
  MQItem *tail;
  uint64_t size;
  uint64_t memory_usage;
  pthread_rwlock_t lock;
};

typedef struct MessageQueueCursor MQCursor;
struct MessageQueueCursor {
  MQItem *item;
  MQ *queue;
};

MQ* MQOpen();
void MQClose(MQ *queue);
void MQPush(MQ *queue, const sds topic, const sds content);

uint64_t MQSize(MQ* queue);
uint64_t MQMemoryUsage(MQ *queue);

MQCursor *MQCreateCursor(MQ *queue);
void MQFreeCursor(MQCursor *cursor);
Message *MQNext(MQCursor *iter);

uint64_t MQFreeOldMessages(MQ *queue, MQItem **last_positions, int len);

#endif  // __KIDS_MSGQUEUE_H_
