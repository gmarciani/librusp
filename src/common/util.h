#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>

char *getTime(void);

char *getUserInput(const char *descr);

char *stringDuplication(const char *src);

char *stringNDuplication(const char *src, size_t size);

char *stringConcatenation(const char *srcOne, const char *srcTwo);

char **splitStringByDelimiter(const char *src, const char *delim, int *numSubstr);

char **splitStringNByDelimiter(const char *src, const char *delim, const int numSubstr);

char **splitStringBySize(const char *src, const size_t size, int *numSubstr);

char **splitStringBySection(const char *src, const size_t *ssize, const int numsubstr);

char *arraySerialization(char **array, const int numItems, const char *delim);

char **arrayDeserialization(const char *sarray, const char *delim, int *numItems);

#endif /* UTIL_H_ */
