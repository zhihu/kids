#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <pthread.h>
#include <cstdlib>

#include "kids.h"

#ifdef HEAP_PROFILE
#include "google/heap-profiler.h"
#endif

Master *kids;
__thread time_t unixtime;
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

std::string config_file;
ParseContext *ctx;

static void Usage(const char* program_name) {
  printf("Usage: %s [options] <config_file>\n"
         "options:\n"
         "  -c, --config <file>   config file of kids\n"
         "  -d, --daemon          run daemonized\n"
         "  -p, --pid             pid file\n"
         "  -v, --version         show version number\n"
         "  -h, --help            print help and quit\n\n",
         program_name);
  exit(0);
}

void *signal_thread_function(void *arg) {
  int err, signo;
  sigset_t *mask = static_cast<sigset_t *>(arg);
  while (true) {
    err = sigwait(mask, &signo);
    if (err != 0) {
      LogFatal("sigwait error: %s", strerror(err));
    }
    switch (signo) {
      case SIGINT:
      case SIGTERM:
      case SIGPIPE:
        LogInfo("Received signal %s, stopping kids now...", strsignal(signo));
        kids->Stop();
        return NULL;
      case SIGHUP:
        {
          ParseContext *ctx_reloaded = ParseConfigFile(config_file);
          if (!ctx_reloaded->success) {
            LogError("%s\n%s\n", ERR_PARSE_CONF, ctx_reloaded->error);
            break;
          }
          delete ctx;
          ctx = ctx_reloaded;
          LogInfo("Reload file for store config: %s", config_file.c_str());
          kids->ReloadStoreConfig(ctx->conf);
        }
        break;
      case SIGUSR1:
        LogInfo("Rotate logfile", strsignal(signo));
        RotateLogger();
        break;
      default:
        LogFatal("error signal mask");
    }
  }

  return NULL;
}

static pthread_t CreateSignalThread(sigset_t *mask) {
  sigemptyset(mask);
  sigaddset(mask, SIGINT);
  sigaddset(mask, SIGTERM);
  sigaddset(mask, SIGPIPE);
  sigaddset(mask, SIGHUP);
  sigaddset(mask, SIGUSR1);

  int err;
  if ((err = pthread_sigmask(SIG_BLOCK, mask, NULL)) != 0) {
    LogFatal("SIG_BLOCK error: %s", strerror(err));
  }
  pthread_t tid;
  err = pthread_create(&tid, NULL, signal_thread_function, mask);
  if (err != 0) {
    LogFatal("create signal thread error %s", strerror(err));
  }
  return tid;
}

static void daemon() {
  int fd;

  if (fork() != 0) exit(0);  // parent exits

  setsid();  // create a new session

  if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > STDERR_FILENO) close(fd);
  }
}

static const char* const short_options = "c:dvh";
static const struct option long_options[] = {
  { "config", required_argument, NULL, 'c' },
  { "daemon", no_argument, NULL, 'd' },
  { "pid", required_argument, NULL, 'p' },
  { "version", no_argument, NULL, 'v' },
  { "help", no_argument, NULL, 'h' },
  { NULL, 0, NULL, 0 },
};

void WritePID(char *pidfile) {
  char buf[32];
  int64_t pid = getpid();
  int fd = open(pidfile, O_WRONLY|O_CREAT, 0644);
  sprintf(buf, "%lld\n", (long long)pid);
  if (fd > 0 && write(fd, buf, strlen(buf)) != strlen(buf)) {
    LogFatal("write pidfile failed");
  }
}

int main(int argc, char **argv) {
  bool daemonize = false;
  char *pidfile = NULL;
  int opt;

  while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) > 0) {
    switch (opt) {
      case 'c':
        config_file = optarg;
        break;
      case 'd':
        daemonize = true;
        break;
      case 'p':
        pidfile = optarg;
        break;
      case 'v':
        printf("%s\n", VERSION);
        exit(0);
        break;
      case 'h':
      default:
        Usage(argv[0]);
    }
  }

  // assume a non-option arg is a config file name
  if (optind < argc && config_file.empty()) {
    config_file = argv[optind];
  }

  if (config_file.empty()) Usage(argv[0]);

  InitLogger(kDebug, "stderr");

  ParseContext *ctx = ParseConfigFile(config_file);
  if (!ctx->success) {
    LogFatal("%s\n%s\n", ERR_PARSE_CONF, ctx->error);
  }

  if (daemonize) daemon();

  InitLogger(LogLevelFromString(ctx->conf->log_level.c_str()), ctx->conf->log_file.c_str());

  unixtime = time(NULL);

#ifdef HEAP_PROFILE
  HeapProfilerStart("main");
#endif

  sigset_t mask;

  pthread_t tid = CreateSignalThread(&mask);
  Master::Create(ctx->conf);

  LogInfo("Server started, Kids version " VERSION);

  if (pidfile) WritePID(pidfile);
  kids->Start();

#ifdef HEAP_PROFILE
  HeapProfilerDump("main");
  HeapProfilerStop();
#endif

  void *ret;
  pthread_join(tid, &ret);

  delete kids;

  if (pidfile) unlink(pidfile);
  delete ctx;
  return 0;
}
