#ifndef __KIDS_UTIL_H_
#define __KIDS_UTIL_H_

#include <stdlib.h>
#include <unordered_set>
#include <unordered_map>

struct Message;

Message *ParseMessage(const char *data, int size);

int string2ll(char *s, size_t slen, long long *value);
int ll2string(char *s, size_t len, long long value);
int stringmatchlen(const char *pattern, int patternLen, const char *string, int stringLen, int nocase);
int stringmatch(const char *pattern, const char *string, int nocase);
void bytesToHuman(char *s, uint64_t n);
bool ParseSize(const char *str, int *value);
bool ParseTime(const char *str, int *value);

#endif // __KIDS_UTIL_H_
