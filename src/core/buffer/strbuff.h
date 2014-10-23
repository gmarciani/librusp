#ifndef STRBUFF_H_
#define STRBUFF_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../../util/stringutil.h"
#include "../../util/threadutil.h"
#include "../../util/macroutil.h"

#define BUFFSIZE 65535

/* STRING BUFFER STRUCTURE */

typedef struct StrBuff {
	size_t size;
	size_t usrsize;
	char content[BUFFSIZE];

	pthread_rwlock_t rwlock;
	pthread_mutex_t mtx;
	pthread_cond_t insert_cnd;
	pthread_cond_t remove_cnd;
} StrBuff;

/* STRING BUFFER CREATION/DISTRUCTION */

void initializeStrBuff(StrBuff *buff);

void destroyStrBuff(StrBuff *buff);

/* BUFFER SIZE */

size_t getStrBuffSize(StrBuff *buff);

size_t getStrBuffSizeUsr(StrBuff *buff);

size_t allignStrBuffSizeUsr(StrBuff *buff);

/* STRING BUFFER I/O */

size_t lookStrBuff(StrBuff *buff, char *content, const size_t size);

size_t readStrBuff(StrBuff *buff, char *content, const size_t size);

size_t writeStrBuff(StrBuff *buff, const char *content, const size_t size);

size_t popStrBuff(StrBuff *buff, const size_t size);

/* STRING BUFFER WAITING */

size_t waitLookMaxStrBuff(StrBuff *buff, char *content, const size_t size);

#endif /* STRBUFF_H_ */
