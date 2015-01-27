#ifndef __KIDS_FILESYSTEM_H_
#define __KIDS_FILESYSTEM_H_

#include <fcntl.h>

#include <cstdio>
#include <cstdint>

#include <string>
#include <vector>

class File {
 public:
  ~File() { Close(false); }
  static File *Open(const char *filename, const char *mode);
  static File *Open(std::string path, std::string name, bool is_secondary, std::string topic, time_t ti);

  void Close(bool remove_if_empty);
  bool IsOpen() { return is_open_; }
  size_t Size();

  bool Write(const char *data, size_t size, bool prepend_size, bool append_eol);
  bool Flush();
  bool Read(void *data, size_t size);

 private:
  File(FILE *file, const std::string& filename) : file_(file), is_open_(true), filename_(filename) {}

  bool Remove();

  FILE *file_;
  bool is_open_;
  std::string filename_;
};

std::string MakeName(std::string pattern, const struct tm& tm, std::string topic);
std::string FindRoot(std::string pattern);

bool MakePath(const char *path, mode_t mode);

std::string FindOldestFile(const char *path);
void DeleteOldestFile(const char *path);

#endif  // __KIDS_FILESYSTEM_H_
