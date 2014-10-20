#include "strbuff.h"

/* STRING BUFFER CREATION/DISTRUCTION */

StrBuff *createStrBuff(void) {
	StrBuff *buff = NULL;

	if (!(buff = malloc(sizeof(StrBuff))))
		ERREXIT("Cannot allocate memory for string buffer.");

	buff->rwlock = createRWLock();

	buff->mtx = createMutex();

	buff->insert_cnd = createConditionVariable();

	buff->remove_cnd = createConditionVariable();

	buff->size = 0;

	return buff;
}

void freeStrBuff(StrBuff *buff) {

	freeRWLock(buff->rwlock);

	freeMutex(buff->mtx);

	freeConditionVariable(buff->insert_cnd);

	freeConditionVariable(buff->remove_cnd);

	buff->size = 0;

	free(buff);
}

/* BUFFER SIZE */

size_t getStrBuffSize(StrBuff *buff) {
	size_t size;

	lockRead(buff->rwlock);

	size = buff->size;

	unlockRWLock(buff->rwlock);

	return size;
}

/* STRING BUFFER I/O */

char *lookStrBuff(StrBuff *buff, const size_t size) {
	char *str = NULL;
	size_t sizeToLook;

	lockRead(buff->rwlock);

	sizeToLook = (size < buff->size) ? size : buff->size;

	if (!(str = malloc(sizeof(char) * (sizeToLook + 1))))
		ERREXIT("Cannot allocate memory for buffer get.");

	memcpy(str, buff->content, sizeof(char) * sizeToLook);

	unlockRWLock(buff->rwlock);

	str[sizeToLook] = '\0';

	return str;
}

char *readStrBuff(StrBuff *buff, const size_t size) {
	char *str = NULL;

	str = lookStrBuff(buff, size);

	popStrBuff(buff, strlen(str));

	return str;
}

void writeStrBuff(StrBuff *buff, const char *str, const size_t size) {
	if (size == 0)
		return;

	lockWrite(buff->rwlock);

	memcpy(buff->content + buff->size, str, sizeof(char) * size);

	buff->size += size;

	unlockRWLock(buff->rwlock);

	broadcastConditionVariable(buff->insert_cnd);
}

void popStrBuff(StrBuff *buff, const size_t size) {
	size_t sizeToPop;

	sizeToPop = (size < buff->size) ? size : buff->size;

	lockWrite(buff->rwlock);

	if (sizeToPop != 0)
		memmove(buff->content, buff->content + sizeToPop, sizeof(char) * (buff->size - sizeToPop));

	buff->size -= sizeToPop;

	unlockRWLock(buff->rwlock);

	broadcastConditionVariable(buff->remove_cnd);
}

/* STRING BUFFER WAITING */

void waitEmptyStrBuff(StrBuff *buff) {
	lockMutex(buff->mtx);

	while (getStrBuffSize(buff) > 0)
		waitConditionVariable(buff->remove_cnd, buff->mtx);

	unlockMutex(buff->mtx);
}

char *waitLookMaxStrBuff(StrBuff *buff, const size_t size) {
	char *result = NULL;

	lockMutex(buff->mtx);

	while (buff->size == 0)
		waitConditionVariable(buff->insert_cnd, buff->mtx);

	unlockMutex(buff->mtx);

	result = lookStrBuff(buff, size);

	return result;
}

char *waitReadMinStrBuff(StrBuff *buff, const size_t size) {
	char *result = NULL;

	lockMutex(buff->mtx);

	while (buff->size < size)
		waitConditionVariable(buff->insert_cnd, buff->mtx);

	unlockMutex(buff->mtx);

	result = readStrBuff(buff, size);

	return result;
}

/* STRING BUFFER REPRESENTATION */

char *strBuffToString(StrBuff *buff) {
	char *strbuff = NULL;
	char *strcontent = NULL;

	strcontent = lookStrBuff(buff, buff->size);

	if (!(strbuff = malloc(sizeof(char) * (25 + strlen(strcontent) + 1))))
		ERREXIT("Cannot allocate memory for buffer string representation.");

	sprintf(strbuff, "csize:%zu content:%s", strlen(strcontent), strcontent);

	free(strcontent);

	return strbuff;
}

