#ifndef __KIDS_SPINLOCK_H_
#define __KIDS_SPINLOCK_H_
#include <atomic>

class Spinlock{
 private:
  std::atomic_flag flag_;
 public:
  Spinlock(): flag_(ATOMIC_FLAG_INIT) {}
  inline void lock() noexcept {
    while (flag_.test_and_set(std::memory_order_acquire)) {}
  }
  inline bool try_lock() noexcept {
    return !flag_.test_and_set(std::memory_order_acquire);
  }

  inline void unlock() noexcept {
    flag_.clear(std::memory_order_release);
  }
};

#endif  /* __KIDS_SPINLOCK_H_ */
