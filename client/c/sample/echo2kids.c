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
  char logbuf[10240];
  int opt;
  int daemon;
  struct timeval timeout = { 1, 500000 }; // 1.5 seconds

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
      case 'd':
        daemon = 1;
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

  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  while (1) {
    if ((linelen = getline(&line, &linecap, stdin)) <= 0) {
      break;
    }
    line[linelen - 1] = '\0';
    reply = redisCommand(c, "PUBLISH %s %s", topic, line);
    if (reply) {
      freeReplyObject(reply);
    } else {
      printf("LOG %s : failed\n", logbuf);
      return -1;
    }

  }

  redisFree(c);
}
