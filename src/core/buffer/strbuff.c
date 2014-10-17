#include "strbuff.h"

/* STRING BUFFER CREATION/DISTRUCTION */

StrBuff *createStrBuff(void) {
	StrBuff *buff = NULL;

	if (!(buff = malloc(sizeof(StrBuff))))
		ERREXIT("Cannot allocate memory for string buffer.");

	buff->mtx = createMutex();

	buff->insert_cnd = createConditionVariable();

	buff->remove_cnd = createConditionVariable();

	buff->size = 0;

	return buff;
}

void freeStrBuff(StrBuff *buff) {

	destroyMutex(buff->mtx);

	destroyConditionVariable(buff->insert_cnd);

	destroyConditionVariable(buff->remove_cnd);

	buff->size = 0;

	free(buff);
}

/* STRING BUFFER I/O */

char *lookStrBuff(StrBuff *buff, const size_t size) {
	char *str = NULL;
	size_t sizeToCopy;

	sizeToCopy = (size < buff->size) ? size : buff->size;

	if (!(str = malloc(sizeof(char) * (sizeToCopy + 1))))
		ERREXIT("Cannot allocate memory for buffer get.");

	memcpy(str, buff->content, sizeof(char) * sizeToCopy);

	str[sizeToCopy] = '\0';

	return str;
}

char *readStrBuff(StrBuff *buff, const size_t size) {
	char *str = NULL;
	size_t sizeToCopy;

	sizeToCopy = (size < buff->size) ? size : buff->size;

	str = lookStrBuff(buff, sizeToCopy);

	if (sizeToCopy != 0)
		memmove(buff->content, buff->content + sizeToCopy, sizeof(char) * (buff->size - sizeToCopy));

	buff->size -= sizeToCopy;

	return str;
}

void writeStrBuff(StrBuff *buff, const char *str, const size_t size) {

	if (size == 0)
		return;

	memcpy(buff->content + buff->size, str, sizeof(char) * size);

	buff->size += size;
}

/* STRING BUFFER REPRESENTATION */

char *strBuffToString(StrBuff *buff) {
	char *strbuff = NULL;
	char *strcontent = NULL;

	strcontent = lookStrBuff(buff, buff->size);

	if (!(strbuff = malloc(sizeof(char) * (25 + strlen(strcontent) + 1))))
		ERREXIT("Cannot allocate memory for buffer string representation.");

	sprintf(strbuff, "csize:%zu content:%s", buff->size, strcontent);

	free(strcontent);

	return strbuff;
}

