#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <string>

#include "logger.h"

static LogLevel log_level = kDebug;
static char *log_filename = NULL;
static FILE *log_file = NULL;
static char now_str[sizeof("2011/11/11 11:11:11")];
static const char *LevelName[] = {
  "NONE",
  "FATAL",
  "CRITICAL",
  "ERROR",
  "WARNING",
  "INFO",
  "DEBUG",
};

static void UpdateTime() {
  static time_t now = 0;
  time_t t = time(NULL);

  //update time every second
  if (t - now == 0) return;
  now = t;

  struct tm tm;
  localtime_r(&now, &tm);
  sprintf(now_str, "%04d/%02d/%02d %02d:%02d:%02d",
      1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
      tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void LogPrint(LogLevel level, const char *fmt, ...) {
  va_list  args;
  if (level > log_level) return;
  va_start(args, fmt);
  if (log_file) vfprintf(log_file, fmt, args);
  va_end(args);
  fflush(log_file);
}

void LogInternal(LogLevel level, const char *fmt, ...) {
  va_list  args;
  if (level > log_level) return;
  UpdateTime();
  if (log_file) fprintf(log_file, "%s [%s] ", now_str, LevelName[level]);
  va_start(args, fmt);
  if (log_file) vfprintf(log_file, fmt, args);
  va_end(args);
  fflush(log_file);
}

LogLevel LogLevelFromString(const char *level) {
  for (int i = kNone; i <= kDebug; i++) {
    if (strcasecmp(LevelName[i], level) == 0) {
      return LogLevel(i);
    }
  }
  return kNone;
}

void InitLogger(LogLevel level, const char *filename) {
  // disable logging if filename is NULL
  if (filename == NULL) return;

  log_level = level;
  if (log_filename != NULL && log_filename != filename) {
    free(log_filename);
  }

  log_filename = strdup(filename);
  if (strcmp(log_filename, "stderr") == 0 || strcmp(log_filename, "") == 0) {
    log_file = stderr;
  } else if (strcmp(log_filename, "stdout") == 0) {
    log_file = stdout;
  } else {
    log_file = fopen(log_filename, "a+");
  }
}

void RotateLogger() {
  pthread_mutex_lock(&log_lock);

  if (log_file != stderr && log_file != stdout && log_file != NULL) {
    char filename[1024];
    struct tm tm;
    time_t now = time(NULL);
    localtime_r(&now, &tm);
    sprintf(filename, "%s-%04d-%02d-%02d-%02d-%02d-%02d",
            log_filename,
            1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    fclose(log_file);
    log_file = NULL;
    rename(log_filename, filename);
  }
  char *filename = log_filename;
  InitLogger(log_level, filename);
  free(filename);

  pthread_mutex_unlock(&log_lock);
}
