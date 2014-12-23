#ifndef __KIDS_SPINLOCK_H_
#define __KIDS_SPINLOCK_H_
#include <atomic>

class Spinlock {
 private:
  std::atomic_flag flag;
 public:
  Spinlock(): flag(ATOMIC_FLAG_INIT) {}
  void Lock() {
    while (flag.test_and_set(std::memory_order_acquire)) {}
  }
  void Unlock() {
    flag.clear(std::memory_order_release);
  }
};

#endif  /* __KIDS_SPINLOCK_H_ */
