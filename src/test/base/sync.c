#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include "../../util/threadutil.h"

#define MAX_READERS 10
#define MAX_WRITERS 5
#define SLEEP_TIME 1000000

static pthread_t READERS[MAX_READERS];
static pthread_t WRITERS[MAX_WRITERS];

typedef struct MyStruct {
	int value;
	int readers;
	int writers;
	pthread_mutex_t *readers_mtx;
	pthread_mutex_t *writers_mtx;
	pthread_rwlock_t *rwlock;
} MyStruct;

static void *readFunc(void *arg);

static void *writeFunc(void *arg);

static int findReaderByTid(const pthread_t tid);

static int findWriterByTid(const pthread_t tid);

int main(void) {
	MyStruct mystruct = {.value = 0, .readers = 0, .writers = 0, .readers_mtx = createMutex(), .writers_mtx = createMutex(), .rwlock = createRWLock()};
	int i;

	for (i = 0; i < MAX_READERS; i++)
		READERS[i] = createThread(readFunc, &mystruct, THREAD_JOINABLE);

	for (i = 0; i < MAX_WRITERS; i++)
		WRITERS[i] = createThread(writeFunc, &mystruct, THREAD_JOINABLE);

	for (i = 0; i < MAX_WRITERS; i++)
		joinThread(WRITERS[i]);

	for (i = 0; i < MAX_READERS; i++)
		joinThread(READERS[i]);

	freeRWLock(mystruct.rwlock);

	freeMutex(mystruct.readers_mtx);

	freeMutex(mystruct.writers_mtx);

	exit(EXIT_SUCCESS);
}

static void *readFunc(void *arg) {
	MyStruct *mystruct = (MyStruct *) arg;

	while (1) {
		lockRead(mystruct->rwlock);

		assert(mystruct->writers == 0);

		lockMutex(mystruct->readers_mtx);

		mystruct->readers++;

		unlockMutex(mystruct->readers_mtx);

		if (mystruct->value >= 20) {

			lockMutex(mystruct->readers_mtx);

			mystruct->readers--;

			unlockMutex(mystruct->readers_mtx);

			unlockRWLock(mystruct->rwlock);

			break;
		}

		printf("READER(%d) reading [readers %d writers %d] value %d\n", findReaderByTid(pthread_self()), mystruct->readers, mystruct->writers, mystruct->value);

		usleep(SLEEP_TIME);

		lockMutex(mystruct->readers_mtx);

		mystruct->readers--;

		unlockMutex(mystruct->readers_mtx);

		assert(mystruct->writers == 0);

		unlockRWLock(mystruct->rwlock);

		usleep(SLEEP_TIME);
	}

	printf("TERMINATING READER(%d)\n", findReaderByTid(pthread_self()));

	return NULL;
}

static void *writeFunc(void *arg) {
	MyStruct *mystruct = (MyStruct *) arg;

	while (1) {
		lockWrite(mystruct->rwlock);

		assert(mystruct->writers == 0);

		assert(mystruct->readers == 0);

		lockMutex(mystruct->writers_mtx);

		mystruct->writers++;

		unlockMutex(mystruct->writers_mtx);

		if (mystruct->value >= 20) {

			lockMutex(mystruct->writers_mtx);

			mystruct->writers--;

			unlockMutex(mystruct->writers_mtx);

			unlockRWLock(mystruct->rwlock);

			break;
		}

		mystruct->value++;

		printf("WRITER(%d) writing [readers %d writers %d] new value %d\n", findWriterByTid(pthread_self()), mystruct->readers, mystruct->writers, mystruct->value);

		lockMutex(mystruct->writers_mtx);

		mystruct->writers--;

		unlockMutex(mystruct->writers_mtx);

		assert(mystruct->writers == 0);

		assert(mystruct->readers == 0);

		unlockRWLock(mystruct->rwlock);

		usleep(SLEEP_TIME);
	}

	printf("TERMINATING WRITER(%d)\n", findWriterByTid(pthread_self()));

	return NULL;
}

static int findReaderByTid(const pthread_t tid) {
	int i;

	for (i = 0; i < MAX_READERS; i++) {
		if (READERS[i] == tid)
			return i;
	}

	return -1;
}

static int findWriterByTid(const pthread_t tid) {
	int i;

	for (i = 0; i < MAX_WRITERS; i++) {
		if (WRITERS[i] == tid)
			return i;
	}

	return -1;
}
