#include <stdio.h>
#include <stdlib.h>
#include "../../util/timeutil.h"
#include "../../util/threadutil.h"

static pthread_mutex_t *MTX;

static pthread_rwlock_t *RWL;

static pthread_spinlock_t *SPL;

static unsigned long long VALUE;

static void *readerMTXFunc(void *arg);

static void *writerMTXFunc(void *arg);

static void *readerRWLFunc(void *arg);

static void *writerRWLFunc(void *arg);

static void *readerSPLFunc(void *arg);

static void *writerSPLFunc(void *arg);

static void profileMTX(unsigned long long read, unsigned long long write);

static void profileRWL(unsigned long long read, unsigned long long write);

static void profileSPL(unsigned long long read, unsigned long long write);

int main(int argc, char **argv) {
	long double read, write;
	int i;

	if (argc < 2 || argc % 2 == 0)
		ERREXIT("usage: %s [(reders, writers), ...]", argv[0]);

	MTX = createMutex();

	RWL = createRWLock();

	SPL = createSpinLock();

	for (i = 1; i < argc; i+=2) {

		read = strtoull(argv[i], NULL, 10);

		write = strtoull(argv[i + 1], NULL, 10);

		profileMTX(read, write);

		profileRWL(read, write);

		profileSPL(read, write);
	}

	freeMutex(MTX);

	freeRWLock(RWL);

	freeSpinLock(SPL);

	exit(EXIT_SUCCESS);
}

static void profileMTX(unsigned long long read, unsigned long long write) {
	pthread_t READER;
	pthread_t WRITER;

	printf("# Profiling MTX on %llu reads and %llu writes...", read, write);

	READER = createThread(readerMTXFunc, &read, THREAD_JOINABLE);

	WRITER = createThread(writerMTXFunc, &write, THREAD_JOINABLE);

	joinThread(READER);

	joinThread(WRITER);

	printf("OK\n");
}

static void profileRWL(unsigned long long read, unsigned long long write) {
	pthread_t READER;
	pthread_t WRITER;

	printf("# Profiling RWL on %llu reads and %llu writes...", read, write);

	READER = createThread(readerRWLFunc, &read, THREAD_JOINABLE);

	WRITER = createThread(writerRWLFunc, &write, THREAD_JOINABLE);

	joinThread(READER);

	joinThread(WRITER);

	printf("OK\n");
}

static void profileSPL(unsigned long long read, unsigned long long write) {
	pthread_t READER;
	pthread_t WRITER;

	printf("# Profiling SPL on %llu reads and %llu writes...", read, write);

	READER = createThread(readerSPLFunc, &read, THREAD_JOINABLE);

	WRITER = createThread(writerSPLFunc, &write, THREAD_JOINABLE);

	joinThread(READER);

	joinThread(WRITER);

	printf("OK\n");
}

static void *readerMTXFunc(void *arg) {
	unsigned long long iterations = *(unsigned long long *) arg;
	unsigned long long i, value;

	for (i = 0; i < iterations; i++) {
		lockMutex(MTX);
		value += VALUE / 2;
		unlockMutex(MTX);
		usleep(10);
	}

	return NULL;
}

static void *writerMTXFunc(void *arg) {
	unsigned long long iterations = *(unsigned long long *) arg;
	unsigned long long i;

	for (i = 0; i < iterations; i++) {
		lockMutex(MTX);
		VALUE++;
		unlockMutex(MTX);
		usleep(10);
	}

	return NULL;
}

static void *readerRWLFunc(void *arg) {
	unsigned long long iterations = *(unsigned long long *) arg;
	unsigned long long i, value;

	for (i = 0; i < iterations; i++) {
		lockRead(RWL);
		value += VALUE / 2;
		unlockRWLock(RWL);
		usleep(10);
	}

	return NULL;
}

static void *writerRWLFunc(void *arg) {
	unsigned long long iterations = *(unsigned long long *) arg;
	unsigned long long i;

	for (i = 0; i < iterations; i++) {
		lockWrite(RWL);
		VALUE++;
		unlockRWLock(RWL);
		usleep(10);
	}

	return NULL;
}

static void *readerSPLFunc(void *arg) {
	unsigned long long iterations = *(unsigned long long *) arg;
	unsigned long long i, value;

	for (i = 0; i < iterations; i++) {
		lockSpinLock(SPL);
		value += VALUE / 2;
		unlockSpinLock(SPL);
		usleep(10);
	}

	return NULL;
}

static void *writerSPLFunc(void *arg) {
	unsigned long long iterations = *(unsigned long long *) arg;
	unsigned long long i;

	for (i = 0; i < iterations; i++) {
		lockSpinLock(SPL);
		VALUE++;
		unlockSpinLock(SPL);
		usleep(10);
	}

	return NULL;
}
