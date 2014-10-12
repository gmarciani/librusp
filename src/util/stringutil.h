#ifndef STRINGUTIL_H_
#define STRINGUTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>

#include "threadutil.h"

#define BUFFSIZE 65535

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

typedef struct Buffer {
	char content[BUFFSIZE];
	size_t size;
	pthread_mutex_t *mtx;
	pthread_cond_t *cnd;
} Buffer;

/* BUFFER MANAGEMENT */

Buffer *createBuffer(void);

void freeBuffer(Buffer *buff);

char *lookBuffer(Buffer *buff, const size_t size);

void popBuffer(Buffer *buff, const size_t size);

char *readBuffer(Buffer *buff, const size_t size);

void writeBuffer(Buffer *buff, const char *str, const size_t size);

char *bufferToString(Buffer *buff);

/* STRING MANAGEMENT */

char *stringDuplication(const char *src);

char *stringNDuplication(const char *src, const size_t size);

char *stringConcatenation(const char *srcone, const char *srctwo);

/* STRING SPLITTING */

char **splitStringByDelimiter(const char *src, const char *delim, int *substrs);

char **splitStringNByDelimiter(const char *src, const char *delim, const int substrs);

char **splitStringBySize(const char *src, const size_t size, int *substrs);

char **splitStringBySection(const char *src, const size_t *ssize, const int substrs);

/* ARRAY (DE)SERIALIZATION */

char *arraySerialization(char **array, const int items, const char *delim);

char **arrayDeserialization(const char *sarray, const char *delim, int *items);

/* VARIOUS */

char *getTime(void);

char *getUserInput(const char *descr);

#endif /* STRINGUTIL_H_ */
