#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include "filesystem.h"
#include "constants.h"
#include "logger.h"

static int DoMakeDIR(const char *path, mode_t mode) {
  struct stat st;
  int status = 0;

  if (stat(path, &st) != 0) {
    // Directory does not exist 
    if (mkdir(path, mode) != 0) {
      LogDebug("make dir %s failed", path);
      status = -1;
    } else {
      LogDebug("make dir %s", path);
    }
  } else if (!S_ISDIR(st.st_mode)) {
    LogError("%s of %s: ", ERR_CREATE_DIR, path, strerror(errno));
    status = -1;
  }

  return status;
}

// MakePath - ensure all directories in path exist
// Algorithm takes the pessimistic view and works top-down to ensure
// each directory in path exists, rather than optimistically creating
// the last element and working backwards.
bool MakePath(const char *path, mode_t mode) {
  char *pp;
  char *sp;
  int status;
  char *copypath = strdup(path);

  status = 0;
  pp = copypath;

  while (status == 0 && (sp = strchr(pp, '/')) != 0) {
    if (sp != pp) {
      // Neither root nor double slash in path
      *sp = '\0';
      status = DoMakeDIR(copypath, mode);
      *sp = '/';
    }
    pp = sp + 1;
  }

  if (status == 0) status = DoMakeDIR(path, mode);

  free(copypath);
  return status == 0;
}

std::string FindRoot(std::string pattern) {
  size_t pos = pattern.find("[date]");
  if (pos != std::string::npos) pattern = pattern.substr(0, pos);
  pos = pattern.find("[time]");
  if (pos != std::string::npos) pattern = pattern.substr(0, pos);
  pos = pattern.find("[topic]");
  if (pos != std::string::npos) pattern = pattern.substr(0, pos);
  int i;
  for (i = pattern.length() - 1; i >= 0; i--) {
    if (pattern[i] == '/') break;
  }
  if (i < 0) return ".";
  return pattern.substr(0, i + 1);
}

std::string FindOldestFile(const char *path) {
  DIR *dp = opendir(path);
  struct dirent *ent;
  struct stat st;
  time_t min_ctime = -1;
  static char buffer[2048];
  std::string filename;

  if (dp == NULL) {
    LogError("open dir %s failed: %s", path, strerror(errno));
    return std::string();
  }

  while ((ent = readdir(dp)) != NULL) {
    if (strcmp(ent->d_name, ".") ==0)
      continue;
    if (strcmp(ent->d_name, "..") ==0)
      continue;
    sprintf(buffer, "%s%s", path, ent->d_name);
    if (stat(buffer, &st) == -1)
      continue;
    if (st.st_ctime < min_ctime || min_ctime == -1) {
      min_ctime = st.st_ctime;
      filename = buffer;
    }
  }

  LogDebug("find old file in %s result: %s", path, filename.c_str());

  closedir(dp);
  return filename;
}

void DeleteOldestFile(const char *path) {
  std::string filename = FindOldestFile(path);
  if (!filename.empty()) {
    LogDebug("unlink file %s", filename.c_str());
    unlink(filename.c_str());
  }
}

std::string MakeName(std::string pattern, const struct tm& tm, std::string topic) {
  char buffer[100];

  sprintf(buffer, "%04d-%02d-%02d", 1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday);
  std::string date(buffer);

  sprintf(buffer, "%02d-%02d-%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
  std::string time(buffer);

  size_t pos = pattern.find("[date]");
  if (pos != std::string::npos) pattern = pattern.replace(pos, 6, date);
  pos = pattern.find("[time]");
  if (pos != std::string::npos) pattern = pattern.replace(pos, 6, time);
  pos = pattern.find("[topic]");
  if (pos != std::string::npos) pattern = pattern.replace(pos, 7, topic);

  return pattern;
}

File *File::Open(std::string path, std::string name, bool is_secondary, std::string topic, time_t ti) {
  time_t now = ti;
  struct tm tm;

  if (now == 0) now = time(NULL);
  localtime_r(&now, &tm);

  std::string pathname = path;
  if (is_secondary) name = "[date]-[time].kidsbuffer";
  if (!is_secondary) pathname = MakeName(path, tm, topic);
  std::string filename = MakeName(name, tm, topic);

  if (MakePath(pathname.c_str(), 0755)) {
    std::string fullname = pathname + filename;
    LogDebug("create file %s", fullname.c_str());
    return Open(fullname.c_str(), "a+");
  }
  return NULL;
}

File *File::Open(const char *filename, const char *mode) {
  if (filename == NULL || mode == NULL) {
    LogError("openning file failed, filename or mode is NULL");
    return NULL;
  }
  FILE *file = fopen(filename, mode);
  LogDebug("openning %s", filename);
  if (file == NULL) {
    LogError("openning file failed %s: %s", filename, strerror(errno));
  } else {
    return new File(file, filename);
  }
  return NULL;
}

void File::Close(bool remove_if_empty) {
  if (!is_open_) return;
  LogDebug("close file %d", fileno(file_));
  fclose(file_);
  file_ = NULL;
  is_open_ = false;
  if (Size() == 0 && remove_if_empty) Remove();
}

size_t File::Size() {
  struct stat st;
  stat(filename_.c_str(), &st);
  return st.st_size;
}

bool File::Read(void *data, size_t size) {
  int ret = fread(data, size, 1, file_);
  if (size == 0 || ret == 0) return false;
  if (ret == -1) {
    LogError("%s(%s):%s", ERR_READ_FILE, filename_.c_str(), strerror(errno));
    Close(false);
    return false;
  }
  return true;
}

bool File::Flush() {
  if (!is_open_) return false;
  int ret = fflush(file_);
  if (ret == 0) {
    LogDebug("%s flushed", filename_.c_str());
    return true;
  } else {
    LogError("%s failed to flush, ret = %d", filename_.c_str(), ret);
    return false;
  }
}

bool File::Write(const char *data, size_t size, bool prepend_size, bool append_eol) {
  int ret = 0;

  // try to reopen if closed by previous error
  if (!is_open_) {
    file_ = fopen(filename_.c_str(), "a+");
    if (file_ == NULL) goto ERROR;
    is_open_ = true;
  }

  if (prepend_size) {
    ret = fwrite(&size, sizeof(int), 1, file_);
    if (ret == -1) goto ERROR;
  }

  ret = fwrite(data, size, 1, file_);
  if (ret == -1) goto ERROR;

  if (append_eol) {
    ret = fwrite("\n", sizeof(char), 1, file_);
    if (ret == -1) goto ERROR;
  }

  return true;

ERROR:
  LogError("%s of %s(%d):%s", ERR_WRITE_FILE, filename_.c_str(), fileno(file_), strerror(errno));
  Close(false);
  return false;
}

bool File::Remove() {
  if (!filename_.empty()) {
    LogDebug("remove file %s", filename_.c_str());
    int ret = unlink(filename_.c_str());
    if (ret == -1) {
      LogDebug("remove file failed of %s: %s", filename_.c_str(), strerror(errno));
      return false;
    }
  }
  return true;
}
