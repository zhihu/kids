#include <cstdio>
#include <cstring>
#include <climits>
#include <cinttypes>

#include <string>

#include "kids.h"
#include "util.h"

Message *ParseMessage(const char *data, int size) {
  sds topic;
  sds content;
  const char *p = strchr(data, '|');
  if (p == NULL) {
    topic = sdsnewlen("zhihu.notopic", 13);
    content = sdsnewlen(data, size);
  } else {
    topic = sdsnewlen(data, p - data);
    content = sdsnewlen(p + 1, size - (p-data) - 1);
  }
  return new Message(topic, content);
}

/* Convert a string into a long long. Returns 1 if the string could be parsed
 * into a (non-overflowing) long long, 0 otherwise. The value will be set to
 * the parsed value when appropriate. */
int string2ll(char *s, size_t slen, long long *value) {
  char *p = s;
  size_t plen = 0;
  int negative = 0;
  unsigned long long v;

  if (plen == slen)
    return 0;

  /* Special case: first and only digit is 0. */
  if (slen == 1 && p[0] == '0') {
    if (value != NULL) *value = 0;
    return 1;
  }

  if (p[0] == '-') {
    negative = 1;
    p++; plen++;

    /* Abort on only a negative sign. */
    if (plen == slen)
      return 0;
  }

  /* First digit should be 1-9, otherwise the string should just be 0. */
  if (p[0] >= '1' && p[0] <= '9') {
    v = p[0]-'0';
    p++; plen++;
  } else if (p[0] == '0' && slen == 1) {
    *value = 0;
    return 1;
  } else {
    return 0;
  }

  while (plen < slen && p[0] >= '0' && p[0] <= '9') {
    if (v > (ULLONG_MAX / 10)) /* Overflow. */
      return 0;
    v *= 10;

    if (v > (ULLONG_MAX - (p[0]-'0'))) /* Overflow. */
      return 0;
    v += p[0]-'0';

    p++; plen++;
  }

  /* Return if no all bytes were used. */
  if (plen < slen)
    return 0;

  if (negative) {
    if (v > ((unsigned long long)(-(LLONG_MIN+1))+1)) /* Overflow. */
      return 0;
    if (value != NULL) *value = -v;
  } else {
    if (v > LLONG_MAX) /* Overflow. */
      return 0;
    if (value != NULL) *value = v;
  }
  return 1;
}

int stringmatchlen(const char *pattern, int patternLen, const char *string, int stringLen, int nocase) {
  while (patternLen) {
    switch (pattern[0]) {
      case '*':
        while (pattern[1] == '*') {
          pattern++;
          patternLen--;
        }
        if (patternLen == 1)
          return 1; /* match */
        while (stringLen) {
          if (stringmatchlen(pattern+1, patternLen-1, string, stringLen, nocase))
            return 1; /* match */
          string++;
          stringLen--;
        }
        return 0; /* no match */
        break;
      case '?':
        if (stringLen == 0)
          return 0; /* no match */
        string++;
        stringLen--;
        break;
      case '[':
        {
          int no, match;

          pattern++;
          patternLen--;
          no = pattern[0] == '^';
          if (no) {
            pattern++;
            patternLen--;
          }
          match = 0;
          while (1) {
            if (pattern[0] == '\\') {
              pattern++;
              patternLen--;
              if (pattern[0] == string[0])
                match = 1;
            } else if (pattern[0] == ']') {
              break;
            } else if (patternLen == 0) {
              pattern--;
              patternLen++;
              break;
            } else if (pattern[1] == '-' && patternLen >= 3) {
              int start = pattern[0];
              int end = pattern[2];
              int c = string[0];
              if (start > end) {
                int t = start;
                start = end;
                end = t;
              }
              if (nocase) {
                start = tolower(start);
                end = tolower(end);
                c = tolower(c);
              }
              pattern += 2;
              patternLen -= 2;
              if (c >= start && c <= end)
                match = 1;
            } else {
              if (!nocase) {
                if (pattern[0] == string[0])
                  match = 1;
              } else {
                if (tolower((int)pattern[0]) == tolower((int)string[0]))
                  match = 1;
              }
            }
            pattern++;
            patternLen--;
          }
          if (no)
            match = !match;
          if (!match)
            return 0; /* no match */
          string++;
          stringLen--;
          break;
        }
      case '\\':
        if (patternLen >= 2) {
          pattern++;
          patternLen--;
        }
        /* fall through */
      default:
        if (!nocase) {
          if (pattern[0] != string[0])
            return 0; /* no match */
        } else {
          if (tolower((int)pattern[0]) != tolower((int)string[0]))
            return 0; /* no match */
        }
        string++;
        stringLen--;
        break;
    }
    pattern++;
    patternLen--;
    if (stringLen == 0) {
      while (*pattern == '*') {
        pattern++;
        patternLen--;
      }
      break;
    }
  }
  if (patternLen == 0 && stringLen == 0)
    return 1;
  return 0;
}

int stringmatch(const char *pattern, const char *string, int nocase) {
  return stringmatchlen(pattern,strlen(pattern),string,strlen(string),nocase);
}

/* Convert a long long into a string. Returns the number of
 * characters needed to represent the number, that can be shorter if passed
 * buffer length is not enough to store the whole number. */
int ll2string(char *s, size_t len, long long value) {
    char buf[32], *p;
    unsigned long long v;
    size_t l;

    if (len == 0) return 0;
    v = (value < 0) ? -value : value;
    p = buf+31; /* point to the last character */
    do {
        *p-- = '0'+(v%10);
        v /= 10;
    } while (v);
    if (value < 0) *p-- = '-';
    p++;
    l = 32-(p-buf);
    if (l+1 > len) l = len-1; /* Make sure it fits, including the nul term */
    memcpy(s,p,l);
    s[l] = '\0';
    return l;
}

/* Convert an amount of bytes into a human readable string in the form
 * of 100B, 2G, 100M, 4K, and so forth. */

void bytesToHuman(char *s, uint64_t n) {
  double d;

  if (n < 1024) {
    /* Bytes */
    sprintf(s,"%" PRIu64 "B",n);
    return;
  } else if (n < (1024*1024)) {
    d = (double)n/(1024);
    sprintf(s,"%.2fK",d);
  } else if (n < (1024LL*1024*1024)) {
    d = (double)n/(1024*1024);
    sprintf(s,"%.2fM",d);
  } else if (n < (1024LL*1024*1024*1024)) {
    d = (double)n/(1024LL*1024*1024);
    sprintf(s,"%.2fG",d);
  } else if (n < (1024LL*1024*1024*1024*1024)) {
    d = (double)n/(1024LL*1024*1024*1024);
    sprintf(s, "%.2fT", d);
  }
}

bool ParseTime(const char *str, int *value) {
  int i, t = 0;
  for (i = 0; str[i] != '\0'; i++) {
    if (str[i] >= '0' && str[i] <= '9') {
      t = t * 10 + (str[i] - '0');
    } else {
      break;
    }
  }

  if (strcasecmp(str + i, "h") == 0 || strcasecmp(str + i, "hour") == 0) {
    t *= 3600;
  } else if (strcasecmp(str + i, "m") == 0 || strcasecmp(str + i, "min") == 0) {
    t *= 60;
  } else if (strcasecmp(str + i, "s") == 0 || strcasecmp(str + i, "sec") == 0) {
    t *= 1;
  } else if (strcasecmp(str + i, "d") == 0 || strcasecmp(str + i, "day") == 0) {
    t *= 3600 * 24;
  } else {
    return false;
  }

  *value = t;
  return true;
}

bool ParseSize(const char *str, int *value) {
  int i, t = 0;
  for (i = 0; str[i] != '\0'; i++) {
    if (str[i] >= '0' && str[i] <= '9') {
      t = t * 10 + (str[i] - '0');
    } else {
      break;
    }
  }

  if (strcasecmp(str + i, "b") == 0 || strcasecmp(str + i, "byte") == 0) {
    t *= 1;
  } else if (strcasecmp(str + i, "k") == 0 || strcasecmp(str + i, "kb") == 0) {
    t *= 1024;
  } else if (strcasecmp(str + i, "m") == 0 || strcasecmp(str + i, "mb") == 0) {
    t *= 1024 * 1024;
  } else if (strcasecmp(str + i, "g") == 0 || strcasecmp(str + i, "gb") == 0) {
    t *= 1024 * 1024 * 1024;
  } else {
    return false;
  }

  *value = t;
  return true;
}
