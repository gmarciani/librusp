#include "threadutil.h"

/* THREAD */

pthread_t createThread(void *(*threadFunc) (void *), void *arg, const uint8_t mode) {
	pthread_t tid;
	pthread_attr_t attr;

	if (pthread_attr_init(&attr) > 0)
		ERREXIT("Cannot initialize thread attributes.");
	
	if (mode == THREAD_DETACHED) {

		if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) > 0)
			ERREXIT("Cannot set thread attribute.");
	}

	errno = 0;

	if (pthread_create(&tid, &attr, threadFunc, arg) > 0)
		ERREXIT("Cannot create thread: %s", strerror(errno));

	if (pthread_attr_destroy(&attr) > 0)
		ERREXIT("Cannot destroy thread attributes.");

	return tid;
}

void cancelThread(pthread_t tid) {
	errno = 0;

	if (pthread_cancel(tid) > 0)
		ERREXIT("Cannot cancel thread: %s", strerror(errno));
}

void *joinThread(pthread_t tid) {
	void *retval;

	errno = 0;

	if (pthread_join(tid, &retval) > 0)
		ERREXIT("Cannot join thread: %s", strerror(errno));

	return retval;
}

/* MUTEX */

pthread_mutex_t *createMutex() {
	pthread_mutex_t *mtx = NULL;

	if (!(mtx = malloc(sizeof(pthread_mutex_t))))
		ERREXIT("Cannot allocate memory for mutex.");

	if (pthread_mutex_init(mtx, NULL) > 0)
		ERREXIT("Cannot initialize mutex.");

	return mtx;	
}

void freeMutex(pthread_mutex_t *mtx) {

	if (pthread_mutex_destroy(mtx) > 0)
		ERREXIT("Cannot destroy mutex.");

	free(mtx);
}

void lockMutex(pthread_mutex_t *mtx) {

	if (pthread_mutex_lock(mtx) > 0)
		ERREXIT("Cannot lock mutex.");
}

void unlockMutex(pthread_mutex_t *mtx) {

	if (pthread_mutex_unlock(mtx) > 0)
		ERREXIT("Cannot unlock mutex.");
}

/* CONDITION VARIABLE */

pthread_cond_t *createConditionVariable() {
	pthread_cond_t *cnd = NULL;

	if (!(cnd = malloc(sizeof(pthread_cond_t))))
		ERREXIT("Cannot allocate memory for condition variable.");

	if (pthread_cond_init(cnd, NULL) > 0)
		ERREXIT("Cannot initialize condition variable.");

	return cnd;	
}

void freeConditionVariable(pthread_cond_t *cnd) {

	if (pthread_cond_destroy(cnd) > 0)
		ERREXIT("Cannot destroy condition variable.");

	free(cnd);
}

void waitConditionVariable(pthread_cond_t *cnd, pthread_mutex_t *mtx) {

	if (pthread_cond_wait(cnd, mtx) > 0)
		ERREXIT("Cannot wait for condition variable.");
}

void waitTimeoutConditionVariable(pthread_cond_t *cnd, pthread_mutex_t *mtx, const struct timespec timeout) {
	int error;

	if ((error = pthread_cond_timedwait(cnd, mtx, &timeout)) > 0)
		if (error != ETIMEDOUT)
			ERREXIT("Cannot timed wait for condition variable.");
}

void signalConditionVariable(pthread_cond_t *cnd) {
	
	if (pthread_cond_signal(cnd) > 0)
		ERREXIT("Cannot signal condition variable.");
}

void broadcastConditionVariable(pthread_cond_t *cnd) {

	if (pthread_cond_broadcast(cnd) > 0)
		ERREXIT("Cannot broadcast condition variable.");
}

/* READ/WRITE LOCK */

pthread_rwlock_t *createRWLock(void) {
	pthread_rwlock_t *rwlock;

	if (!(rwlock = malloc(sizeof(pthread_rwlock_t))))
		ERREXIT("Cannot allocate memory for read-write lock.");

	if (pthread_rwlock_init(rwlock, NULL) > 0)
		ERREXIT("Cannot initialize read-write lock.");

	return rwlock;
}

void freeRWLock(pthread_rwlock_t *rwlock) {
	if (pthread_rwlock_destroy(rwlock) > 0)
		ERREXIT("Cannot destroy read-write lock.");

	free(rwlock);
}

void lockRead(pthread_rwlock_t *rwlock) {
	int error;

	if ((error = pthread_rwlock_rdlock(rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock: %d", error);
}

void lockWrite(pthread_rwlock_t *rwlock) {
	int error;

	if ((error = pthread_rwlock_wrlock(rwlock)) > 0)
		ERREXIT("Cannot acquire write-lock: %d", error);
}

void unlockRWLock(pthread_rwlock_t *rwlock) {
	int error;

	if ((error = pthread_rwlock_unlock(rwlock)) > 0)
		ERREXIT("Cannot release read-write lock: %d", error);
}

/* SEMAPHORE */

sem_t *createSemaphore(const unsigned int value) {
	sem_t *sem = NULL;

	if (!(sem = malloc(sizeof(sem_t))))
		ERREXIT("Cannot allocate memory for semaphore.");

	if (sem_init(sem, 0, value) != 0)
		ERREXIT("Cannot initialize semaphore.");

	return sem;
}

void freeSemaphore(sem_t *sem) {
	if (sem_destroy(sem) != 0)
		ERREXIT("Cannot destroy semaphore.");

	free(sem);
}

void incrementSemaphore(sem_t *sem) {
	if (sem_post(sem) != 0)
		ERREXIT("Cannot increment semaphore.");
}

void decrementSemaphore(sem_t *sem) {
	if (sem_wait(sem) != 0)
		ERREXIT("Cannot decrement semaphore.");
}
