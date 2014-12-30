#include "msgqueue.h"
#include "logger.h"

MQ* MQOpen() {
  MQ *queue = new MQ;
  queue->head = queue->tail = new MQItem(NULL, NULL, NULL);
  queue->size = 0;
  queue->memory_usage = 0;
  pthread_rwlock_init(&queue->lock, NULL);
  return queue;
}

void MQClose(MQ *queue) {
  MQItem *q = queue->head;
  MQItem *next;
  while (q != NULL) {
    next = q->next;
    delete q;
    q = next;
  }
  pthread_rwlock_destroy(&queue->lock);
  delete queue;
}

void MQPush(MQ *queue, const sds topic, const sds content) {
  MQItem *msg = new MQItem(topic, content, NULL);

  pthread_rwlock_wrlock(&queue->lock);
  queue->tail->next = msg;
  queue->tail = msg;
  queue->size++;
  queue->memory_usage += (sdslen(topic) + sdslen(content));
  pthread_rwlock_unlock(&queue->lock);
}

uint64_t MQSize(MQ* queue) {
  uint64_t size;
  pthread_rwlock_rdlock(&queue->lock);
  size = queue->size;
  pthread_rwlock_unlock(&queue->lock);
  return size;
}

uint64_t MQMemoryUsage(MQ *queue) {
  uint64_t mem_useage;
  pthread_rwlock_rdlock(&queue->lock);
  mem_useage = queue->memory_usage;
  pthread_rwlock_unlock(&queue->lock);
  return mem_useage;
}

MQCursor *MQCreateCursor(MQ *queue) {
  MQCursor *c = new MQCursor;
  c->queue = queue;
  c->item = queue->head;
  return c;
}

void MQFreeCursor(MQCursor *cursor) {
  if (cursor != NULL)
    delete cursor;
}

Message* MQNext(MQCursor *iter) {
  MQItem *next = NULL;

  pthread_rwlock_rdlock(&iter->queue->lock);
  if (iter->item != NULL && iter->item->next != NULL) {
    next = iter->item = iter->item->next;
  }
  pthread_rwlock_unlock(&iter->queue->lock);

  return next == NULL ? NULL : next->value;
}

/* free items older than the oldest message unread
 * returns items in queue now
 */
uint64_t MQFreeOldMessages(MQ *queue, MQItem **last_positions, int len) {
  uint64_t count = 0;
  MQItem *p = queue->head;
  MQItem *next = NULL;

  uint64_t memory_freed = 0;
  while (true) {
    bool done = false;
    for (int i = 0; i < len; ++i) {
      if (p == last_positions[i]) done = true;
    }
    if (done) break;
    if (p->value->topic != nullptr && p->value->content != nullptr)
      memory_freed += (sdslen(p->value->topic) + sdslen(p->value->content));

    next = p->next;
    delete p;
    p = next;
    ++count;
  }

  queue->head = p;

  uint64_t size = 0;
  pthread_rwlock_wrlock(&queue->lock);
  queue->memory_usage -= memory_freed;
  queue->size -= count;
  size = queue->size;
  pthread_rwlock_unlock(&queue->lock);

  return size;
}
