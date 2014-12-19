#include <cstdio>
#include <cstring>
#include <cstdarg>

#include <algorithm>

#include "buffer.h"
#include "logger.h"

bool operator<(const Buffer& lhs, const Buffer& rhs) {
  int res = memcmp(lhs.data_, rhs.data_, (lhs.size_ < rhs.size_? lhs.size_ : rhs.size_));
  if (res != 0) return res < 0;
  return lhs.size_ < rhs.size_;
}

bool operator==(const Buffer& lhs, const Buffer& rhs) {
  if (lhs.size_ != rhs.size_) return false;
  return memcmp(lhs.data_, rhs.data_, lhs.size_) == 0;
}

// == constructors and destructors ==

Buffer::Buffer(const char *data, uint32_t size) {
  data_ = static_cast<char*>(malloc(size + 1));
  memcpy(data_, data, size);
  data_[size] = '\0';
  size_ = size;
  capacity_ = size;
  refcount_ = new uint32_t(1);
}

Buffer::Buffer(uint32_t size) {
  data_ = static_cast<char*>(malloc(size + 1));
  data_[size] = '\0';
  size_ = 0;
  capacity_ = size;
  refcount_ = new uint32_t(1);
}

Buffer::Buffer(const char *str) {
  size_ = strlen(str);
  data_ = static_cast<char*>(malloc(size_ + 1));
  capacity_ = size_;
  memcpy(data_, str, size_ + 1);
  refcount_ = new uint32_t(1);
}

Buffer::Buffer(const Buffer& other) {
  ShallowCopy(other);
}

Buffer::~Buffer() {
  Free();
}

size_t Buffer::hash() const {
  size_t hash = 5381;

  for (int i = 0; i < (signed)size_; i++)
    hash = ((hash << 5) + hash) + data_[i];  // hash * 33 + c
  return hash;
}

// == aux functions ==

void Buffer::ShallowCopy(const Buffer& other) {
  data_ = other.data_;
  size_ = other.size_;
  capacity_ = other.capacity_;
  refcount_ = other.refcount_;
  (*refcount_)++;
}

void Buffer::Free() {
  if ((--(*refcount_)) == 0) {
    free(data_);
    delete refcount_;
  }
  data_ = NULL;
  size_ = 0;
  capacity_ = 0;
  refcount_ = NULL;
}

// if only me holds the data, do nothing
// else deref and make a new copy
void Buffer::Clone() {
  if (*refcount_ == 1) return;
  char *buf = static_cast<char*>(malloc(capacity_ + 1));
  memcpy(buf, data_, size_ + 1);
  data_ = buf;
  --(*refcount_);
  refcount_ = new uint32_t(1);
}

// == user functions ==

const Buffer& Buffer::operator=(const Buffer& other) {
  Free();
  ShallowCopy(other);
  return *this;
}

const Buffer& Buffer::operator=(const char *str) {
  // free old buffer first
  if (*refcount_ > 1) {
    Free();
    refcount_ = new uint32_t(1);
  }

  size_ = strlen(str);
  if (capacity_ < size_ || data_ == NULL) {
    char *buf = static_cast<char*>(realloc(data_, size_ + 1));
    if (buf != NULL) {
      data_ = buf;
      capacity_ = size_;  // maybe size_ + 1
    } else {
      LogError("realloc failed");
    }
  }

  memcpy(data_, str, size_ + 1);

  return *this;
}

// what happens when size > size_
void Buffer::remove_front(uint32_t size) {
  if (size == 0) return;

  Clone();

  // don't forget the trailing NULL
  memmove(data_, data_ + size, size_ - size + 1);
  size_ -= size;

  // shrink
  if (size_ * 2 < capacity_) {
    resize(size_);
  }
}

void Buffer::clear() {
  if (*refcount_ == 1) {
    free(data_);
    delete refcount_;
  } else {
    (*refcount_)--;
  }
  data_ = static_cast<char*>(malloc(1));
  data_[0] = '\0';
  refcount_ = new uint32_t(1);
  capacity_ = 0;
  size_ = 0;
}

Buffer *Buffer::pop_front(uint32_t size) {
  if (size > size_) return NULL;

  Clone();

  Buffer *buf = new Buffer(size);
  memcpy(buf->data_, data_, size);

  // don't forget the trailing NULL
  memmove(data_, data_ + size, size_ - size + 1);
  size_ -= size;

  // shrink
  if (size_ * 2 < capacity_) {
    resize(size_);
  }
  return buf;
}

bool Buffer::append(const char *data, uint32_t size) {
  if (size <= 0) return true;

  Clone();

  if (reserve(size_ + size)) {
    memcpy(data_ + size_, data, size);
    size_ += size;
    data_[size_] = '\0';
    return true;
  }

  LogError("no mem");
  return false;
}

void Buffer::append_printf(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  append_vaprintf(fmt, ap);
  va_end(ap);
}

void Buffer::append_vaprintf(const char *fmt, va_list ap) {
  va_list cpy;
  char *buf = NULL;
  size_t buflen = 16;

  // try the length of printed length
  while (1) {
    buf = static_cast<char *>(malloc(buflen));
    if (buf == NULL) {
      LogError("error append vaprintf: %d", buflen);
      return;
    }
    buf[buflen-2] = '\0';
    va_copy(cpy, ap);
    vsnprintf(buf, buflen, fmt, cpy);
    if (buf[buflen-2] != '\0') {
      free(buf);
      buflen *= 2;
      continue;
    }
    break;
  }

  append(buf, strlen(buf));
  free(buf);
}

// ==internal functions that runs under the condition==
// that i own the data

bool Buffer::reserve(uint32_t size) {
  if (size < capacity_) return true;
  uint32_t s = std::max(2*capacity_, size);
  return resize(s) >= s;
}

uint32_t Buffer::resize(uint32_t size) {
  // +1 means a NULL terminator to make buffer behaive like a string
  char *buf = static_cast<char*>(realloc(data_, size + 1));
  if (buf) {
    data_ = buf;
    capacity_ = size;
    data_[capacity_] = '\0';
  } else {
    LogError("resize error");
  }

  return capacity_;
}
