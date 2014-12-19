#ifndef __KIDS_LOGGER_H_
#define __KIDS_LOGGER_H_

#include <pthread.h>
#include <cstdlib>

enum LogLevel {
  kNone = 0,
  kFatal,
  kCritical,
  kError,
  kWarning,
  kInfo,
  kDebug,
};

extern pthread_mutex_t log_lock;

#define LogInfo(s...) do {\
  pthread_mutex_lock(&log_lock);\
  LogInternal(kInfo, s);\
  LogPrint(kInfo, "\n"); \
  pthread_mutex_unlock(&log_lock);\
}while(0)

#define LogWarning(s...) do {\
  pthread_mutex_lock(&log_lock);\
  LogInternal(kWarning, s);\
  LogPrint(kWarning, "\n"); \
  pthread_mutex_unlock(&log_lock);\
}while(0)

#define LogError(s...) do {\
  pthread_mutex_lock(&log_lock);\
  LogInternal(kError, s);\
  LogPrint(kError, "\n"); \
  pthread_mutex_unlock(&log_lock);\
}while(0)

#define LogCritical(s...) do {\
  pthread_mutex_lock(&log_lock);\
  LogInternal(kCritical, s);\
  LogPrint(kCritical, "\n"); \
  pthread_mutex_unlock(&log_lock);\
}while(0)

#define LogFatal(s...) do {\
  pthread_mutex_lock(&log_lock);\
  LogInternal(kFatal, s);\
  LogPrint(kFatal, "\n"); \
  pthread_mutex_unlock(&log_lock);\
  exit(-1);\
}while(0)

#ifdef LOGDEBUG
#define LogDebug(s...) do {\
  pthread_mutex_lock(&log_lock);\
  LogInternal(kDebug, s);\
  LogPrint(kDebug, " [%s]", __PRETTY_FUNCTION__);\
  LogPrint(kDebug, "\n"); \
  pthread_mutex_unlock(&log_lock);\
}while(0)
#else
#define LogDebug(s...)
#endif

void InitLogger(LogLevel level, const char *filename);
LogLevel LogLevelFromString(const char *level);
void RotateLogger();

void LogInternal(LogLevel level, const char *fmt, ...);
void LogPrint(LogLevel level, const char *fmt, ...);

#endif  // __KIDS_LOGGER_H_
