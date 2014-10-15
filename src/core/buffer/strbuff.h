#ifndef STRBUFF_H_
#define STRBUFF_H_

#include "../../util/stringutil.h"
#include "../../util/threadutil.h"
#include "../../util/macroutil.h"

#define BUFFSIZE 65535

/* STRING BUFFER STRUCTURE */

typedef struct Buffer {
	char content[BUFFSIZE];
	size_t size;
	pthread_mutex_t *mtx;
	pthread_cond_t *insert_cnd;
	pthread_cond_t *remove_cnd;
} Buffer;

/* CREATION/DISTRUCTION */

Buffer *createBuffer(void);

void freeBuffer(Buffer *buff);

/* I/O */

char *lookBuffer(Buffer *buff, const size_t size);

char *readBuffer(Buffer *buff, const size_t size);

void writeBuffer(Buffer *buff, const char *str, const size_t size);

/* REPRESENTATION */

char *bufferToString(Buffer *buff);

#endif /* STRBUFF_H_ */
