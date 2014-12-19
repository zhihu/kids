#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

#include "hiredis/hiredis.h"

void Usage(const char *program_name) {
  printf("Usage: %s [options]\n"
      "options:\n"
      "  -s, --socket <file>   unix socket of kids\n"
      "  -h, --host <host>     host of kids (at least one of ip and socket must be specified)\n"
      "  -p, --port <port>     port of kids (default is 3388)\n"
      "  -t, --topic <topic>   topic of log send to kids (default is \"KID_SAMPLE\")\n"
      "  -f, --file <log file> read logs from file\n"
      "  -n, --num <num>       number of logs send to kids (default is -1 means infinite)\n"
      "  -q, --quiet           don't print logs on screen\n"
      "  --period <time>       time (millisecond) between sending two logs\n"
      "  -h, --help            print help and quit\n\n",
      program_name);
  exit(0);
}

const char* const short_options = "s:h:p:t:f:n:qH";
const struct option long_options[] = {
  { "socket", required_argument, NULL, 's' },
  { "host", required_argument, NULL, 'h' },
  { "port", required_argument, NULL, 'p' },
  { "topic", required_argument, NULL, 't' },
  { "file", required_argument, NULL, 'f' },
  { "num", required_argument, NULL, 'n' },
  { "period", required_argument, NULL, 'r' },
  { "quiet", no_argument, NULL, 'q' },
  { "help", no_argument, NULL, 'H' },
  { NULL, 0, NULL, 0 },
};

int main(int argc, char **argv) {
  redisContext *c;
  redisReply *reply;
  const char *socket = NULL;
  const char *ip = "127.0.0.1";
  int port = 3388;
  const char *topic = "loggen";
  const char *filename = NULL;
  FILE *logfile = NULL;
  int num = -1;
  int period = -1;
  int i = 0;
  void *kid = NULL;
  char logbuf[10240];
  int opt;
  int quiet = 0;
  struct timeval timeout = { 1, 500000 }; // 1.5 seconds
  struct timeval start, end;

  while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) > 0) {
    switch (opt) {
      case 's':
        socket = optarg;
        break;
      case 'h':
        ip = optarg;
        break;
      case 'p':
        port = atoi(optarg);
        break;
      case 't':
        topic = optarg;
        break;
      case 'f':
        filename = optarg;
        break;
      case 'n':
        num = atoi(optarg);
        break;
      case 'r':
        period = atoi(optarg);
        break;
      case 'q':
        quiet = 1;
        break;
      case 'H':
      default:
        Usage(argv[0]);
    }
  }

  if (socket == NULL && ip == NULL) {
    Usage(argv[0]);
  }

  if (socket) {
    c = redisConnectUnixWithTimeout(socket, timeout);
  } else {
    c = redisConnectWithTimeout((char*)ip, port, timeout);
  }
  if (c->err) {
    printf("Connection error: %s\n", c->errstr);
    exit(1);
  }

  if (filename != NULL) {
    logfile = fopen(filename, "r");
  }

  while (i < num || num == -1) {
    i++;
    /* log to kids */
    snprintf(logbuf, 1024, "sn %6d", i);
    reply = redisCommand(c, "PUBLISH %s %s", topic, logbuf);
    freeReplyObject(reply);
    /* if (logfile != NULL) { */
    /*   if (fgets(logbuf, 10240, logfile) == NULL) { */
    /*     break; */
    /*   } */
    /* } else { */
    /*   snprintf(logbuf, 1024, "sn %6d", i); */
    /* } */
    /* gettimeofday(&start, NULL); */
    /* gettimeofday(&end, NULL); */
    /* int difs = end.tv_sec - start.tv_sec; */
    /* int difus = end.tv_usec - start.tv_usec; */
    /* if (difus < 0) { */
    /*   difus += 1000000; */
    /*   difs -= 1; */
    /* } */
    /* if (reply) { */
    /*   if (!quiet) printf("LOG %s : %s  %6dms\n", logbuf, reply->str, difs * 1000 + difus/1000); */
    /*   freeReplyObject(reply); */
    /* } else { */
    /*   printf("LOG %s : failed\n", logbuf); */
    /*   return -1; */
    /* } */

    /* if (period != -1) { */
    /*   usleep(period * 1000); */
    /* } */
  }

  if (logfile != NULL) fclose(logfile);
  redisFree(c);
}

