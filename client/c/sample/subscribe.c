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
      "  -i, --ip <ip>         ip of kids (at least one of ip and socket must be specified)\n"
      "  -p, --port <port>     port of kids (default is 3388)\n"
      "  -t, --topic <topic>   topic of log send to kids (default is \"KID_SAMPLE\")\n"
      "  -b, --psub            subscribe a pattern\n"
      "  -n, --num <num>       number of logs send to kids (default is -1 means infinite)\n"
      "  -q, --quiet           don't print logs on screen\n"
      "  --period <time>       time (millisecond) between sending two logs\n"
      "  -h, --help            print help and quit\n\n",
      program_name);
  exit(0);
}

int main(int argc, char **argv) {
  redisContext *c;
  redisReply *reply;
  const char *socket = NULL;
  const char *ip = "127.0.0.1";
  int port = 3388;
  const char *topic = "loggen";
  int num = -1;
  int period = -1;
  int i = 0;
  void *kid = NULL;
  char logbuf[10240];
  int opt;
  int quiet = 0;
  int psub = 0;
  const char* const short_options = "s:i:p:t:f:n:qbh";
  const struct option long_options[] = {
    { "socket", required_argument, NULL, 's' },
    { "ip", required_argument, NULL, 'i' },
    { "port", required_argument, NULL, 'p' },
    { "topic", required_argument, NULL, 't' },
    { "psub", no_argument, NULL, 'b' },
    { "num", required_argument, NULL, 'n' },
    { "period", required_argument, NULL, 'r' },
    { "quiet", no_argument, NULL, 'q' },
    { "help", no_argument, NULL, 'h' },
    { NULL, 0, NULL, 0 },
  };

  while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) > 0) {
    switch (opt) {
      case 's':
        socket = optarg;
        break;
      case 'i':
        ip = optarg;
        break;
      case 'p':
        port = atoi(optarg);
        break;
      case 't':
        topic = optarg;
        break;
      case 'b':
        psub = 1;
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
      case 'h':
      default:
        Usage(argv[0]);
    }
  }

  if (socket == NULL && ip == NULL) {
    Usage(argv[0]);
  }

  struct timeval timeout = { 1, 500000 }; // 1.5 seconds

  if (socket) {
    c = redisConnectUnixWithTimeout(socket, timeout);
  } else {
    c = redisConnectWithTimeout((char*)ip, port, timeout);
  }
  if (c->err) {
    printf("Connection error: %s\n", c->errstr);
    exit(1);
  }

  if (psub) reply = redisCommand(c,"PSUBSCRIBE %s", topic);
  else reply = redisCommand(c,"SUBSCRIBE %s", topic);
  if (reply) {
    printf("SUBSCRIBE %s\n", reply->str);
    freeReplyObject(reply);
  }

  while (1) {
    if (!redisGetReply(c, (void*)&reply)) {
      printf("%s %s %s",  reply->element[0]->str, reply->element[1]->str, reply->element[2]->str);
      if (psub) printf("%s", reply->element[3]->str);
      printf("\n");
      freeReplyObject(reply);
    } else {
      printf("get msg failed\n");
    }

    if (period != -1) {
      usleep(period * 1000);
    }
  }

  redisFree(c);
}


