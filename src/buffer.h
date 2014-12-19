#ifndef __KIDS_BUFFER_H_
#define __KIDS_BUFFER_H_

#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

class Buffer {
  friend bool operator<(const Buffer& lhs, const Buffer& rhs);
  friend bool operator==(const Buffer& lhs, const Buffer& rhs);

 public:
  struct Hasher {
    size_t operator()(const Buffer& buf) const {
      return buf.hash();
    }
  };

  Buffer(const char *data, uint32_t size);
  Buffer(uint32_t size = 0);
  Buffer(const char *str);
  Buffer(const Buffer& other);
  ~Buffer();

  const Buffer& operator=(const Buffer& other);
  const Buffer& operator=(const char *str);
  // readonly operator[]
  char operator[](int index) const { return data_[index]; }

  Buffer *pop_front(uint32_t size);
  void remove_front(uint32_t size);

  char *data(int pos = 0) { return data_ + pos; }
  const char *data(int pos = 0) const { return data_ + pos; }
  size_t hash() const;

  char *blank() { return data_ + size_; }

  uint32_t size() const { return size_; }
  void set_size(uint32_t size) { size_ = size; }
  uint32_t blank_size() const { return capacity_ - size_; }
  uint32_t capacity() const { return capacity_;}

  bool append(const char *data, uint32_t size);
  void clear();
  uint32_t resize(uint32_t size);
  bool reserve(uint32_t size);
  uint32_t refcount() const { return *refcount_; }

  void append_printf(const char *fmt, ...);
  void append_vaprintf(const char *fmt, va_list ap);

 private:
  void ShallowCopy(const Buffer& other);
  void Free();
  void Clone();

  char *data_;
  uint32_t size_;
  uint32_t capacity_;

  uint32_t *refcount_;
};

bool operator<(const Buffer& lhs, const Buffer& rhs);
bool operator==(const Buffer& lhs, const Buffer& rhs);

#endif // __KIDS_BUFFER_H_
