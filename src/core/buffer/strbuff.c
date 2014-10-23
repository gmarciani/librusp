#include "strbuff.h"

void initializeStrBuff(StrBuff *buff) {
	if ((pthread_rwlock_init(&(buff->rwlock), NULL) > 0) |
		(pthread_mutex_init(&(buff->mtx), NULL) > 0) |
		(pthread_cond_init(&(buff->insert_cnd), NULL) > 0) |
		(pthread_cond_init(&(buff->remove_cnd), NULL) > 0))
		ERREXIT("Cannot initialize string buffer sync-block.");

	buff->size = 0;

	buff->usrsize = 0;
}

void destroyStrBuff(StrBuff *buff) {
	if ((pthread_rwlock_destroy(&(buff->rwlock)) > 0) |
		(pthread_mutex_destroy(&(buff->mtx)) > 0) |
		(pthread_cond_destroy(&(buff->insert_cnd)) > 0) |
		(pthread_cond_destroy(&(buff->remove_cnd)) > 0))
		ERREXIT("Cannot destroy string buffer sync-block.");

	buff->size = 0;

	buff->usrsize = 0;
}

/* BUFFER SIZE */

size_t getStrBuffSize(StrBuff *buff) {
	size_t size;

	if (pthread_rwlock_rdlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	size = buff->size;

	if (pthread_rwlock_unlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return size;
}

size_t getStrBuffSizeUsr(StrBuff *buff) {
	size_t size;

	if (pthread_rwlock_rdlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	size = buff->usrsize;

	if (pthread_rwlock_unlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return size;
}

size_t allignStrBuffSizeUsr(StrBuff *buff) {
	size_t size;

	if (pthread_rwlock_wrlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot acquire write-lock.");

	buff->usrsize = buff->size;

	size = buff->usrsize;

	if (pthread_rwlock_unlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return size;
}

/* STRING BUFFER I/O */

size_t lookStrBuff(StrBuff *buff, char *content, const size_t size) {
	size_t looked;

	if (pthread_rwlock_rdlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	looked = MIN(size, buff->size);

	memcpy(content, buff->content, sizeof(char) * looked);

	if (pthread_rwlock_unlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

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

		if (pthread_rwlock_wrlock(&(buff->rwlock)) > 0)
			ERREXIT("Cannot acquire write-lock.");

		memcpy(buff->content + buff->size, content, sizeof(char) * written);

		buff->size += written;

		if (pthread_rwlock_unlock(&(buff->rwlock)) > 0)
			ERREXIT("Cannot release read-write lock.");

		if (pthread_cond_broadcast(&(buff->insert_cnd)) > 0)
			ERREXIT("Cannot broadcast condition variable.");
	}

	return written;
}

size_t popStrBuff(StrBuff *buff, const size_t size) {
	size_t popped;

	popped = MIN(size, buff->size);

	if (popped > 0) {

		if (pthread_rwlock_wrlock(&(buff->rwlock)) > 0)
			ERREXIT("Cannot acquire write-lock.");

		memmove(buff->content, buff->content + popped, sizeof(char) * (buff->size - popped));

		buff->size -= popped;

		buff->usrsize = MAX(0, buff->usrsize - popped);

		if (pthread_rwlock_unlock(&(buff->rwlock)) > 0)
			ERREXIT("Cannot release read-write lock.");

		if (pthread_cond_broadcast(&(buff->remove_cnd)) > 0)
			ERREXIT("Cannot broadcast condition variable.");
	}

	return popped;
}

/* STRING BUFFER WAITING */

size_t waitLookMaxStrBuff(StrBuff *buff, char *content, const size_t size) {
	size_t looked;

	if (pthread_mutex_lock(&(buff->mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (getStrBuffSize(buff) == 0)
		if (pthread_cond_wait(&(buff->insert_cnd), &(buff->mtx)) > 0)
				ERREXIT("Cannot wait for condition variable.");

	if (pthread_mutex_unlock(&(buff->mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");

	looked = lookStrBuff(buff, content, size);

	return looked;
}
