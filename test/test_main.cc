#include "gtest/gtest.h"

#include "kids.h"
#include "logger.h"

Master *kids;
aeEventLoop *serverel;
Statistic statistic;
__thread time_t unixtime;
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

GTEST_API_ int main(int argc, char **argv) {
  InitLogger(kNone, "/tmp/kids.log");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
