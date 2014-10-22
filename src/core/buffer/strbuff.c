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

size_t lookStrBuff(StrBuff *buff, char *content, const size_t size) {
	size_t looked;

	lockRead(buff->rwlock);

	looked = MIN(size, buff->size);

	memcpy(content, buff->content, sizeof(char) * looked);

	unlockRWLock(buff->rwlock);

	return looked;
}

size_t readStrBuff(StrBuff *buff, char *content, const size_t size) {
	size_t read;

	read = lookStrBuff(buff, content, size);

	popStrBuff(buff, read);

	return read;
}

size_t writeStrBuff(StrBuff *buff, const char *content, const size_t size) {
	size_t written;

	written = MIN(size, BUFFSIZE - buff->size);

	if (written > 0) {

		lockWrite(buff->rwlock);

		memcpy(buff->content + buff->size, content, sizeof(char) * written);

		buff->size += written;

		unlockRWLock(buff->rwlock);

		broadcastConditionVariable(buff->insert_cnd);

	}

	return written;
}

size_t popStrBuff(StrBuff *buff, const size_t size) {
	size_t popped;

	popped = MIN(size, buff->size);

	if (popped > 0) {

		lockWrite(buff->rwlock);

		memmove(buff->content, buff->content + popped, sizeof(char) * (buff->size - popped));

		buff->size -= popped;

		unlockRWLock(buff->rwlock);

		broadcastConditionVariable(buff->remove_cnd);
	}

	return popped;
}

/* STRING BUFFER WAITING */

void waitEmptyStrBuff(StrBuff *buff) {
	lockMutex(buff->mtx);

	while (getStrBuffSize(buff) > 0)
		waitConditionVariable(buff->remove_cnd, buff->mtx);

	unlockMutex(buff->mtx);
}

size_t waitLookMaxStrBuff(StrBuff *buff, char *content, const size_t size) {
	size_t looked;

	lockMutex(buff->mtx);

	while (buff->size == 0)
		waitConditionVariable(buff->insert_cnd, buff->mtx);

	unlockMutex(buff->mtx);

	looked = lookStrBuff(buff, content, size);

	return looked;
}

size_t waitReadMinStrBuff(StrBuff *buff, char *content, const size_t size) {
	size_t read;

	lockMutex(buff->mtx);

	while (buff->size < size)
		waitConditionVariable(buff->insert_cnd, buff->mtx);

	unlockMutex(buff->mtx);

	read = readStrBuff(buff, content, size);

	return read;
}

/* STRING BUFFER REPRESENTATION */

char *strBuffToString(StrBuff *buff) {
	char *strbuff = NULL;
	char cnt[buff->size];

	lookStrBuff(buff, cnt, buff->size);

	if (!(strbuff = malloc(sizeof(char) * (25 + buff->size + 1))))
		ERREXIT("Cannot allocate memory for buffer string representation.");

	sprintf(strbuff, "csize:%zu content:%s", buff->size, cnt);

	return strbuff;
}

