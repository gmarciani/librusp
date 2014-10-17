#ifndef STRBUFF_H_
#define STRBUFF_H_

#include "../../util/stringutil.h"
#include "../../util/threadutil.h"
#include "../../util/macroutil.h"

#define BUFFSIZE 65535

/* STRING BUFFER STRUCTURE */

typedef struct StrBuff {
	size_t size;
	char content[BUFFSIZE];

	pthread_mutex_t *mtx;
	pthread_cond_t *insert_cnd;
	pthread_cond_t *remove_cnd;
} StrBuff;

/* STRING BUFFER CREATION/DISTRUCTION */

StrBuff *createStrBuff(void);

void freeStrBuff(StrBuff *buff);

/* STRING BUFFER I/O */

char *lookStrBuff(StrBuff *buff, const size_t size);

char *readStrBuff(StrBuff *buff, const size_t size);

void writeStrBuff(StrBuff *buff, const char *str, const size_t size);

/* STRING BUFFER REPRESENTATION */

char *strBuffToString(StrBuff *buff);

#endif /* STRBUFF_H_ */
