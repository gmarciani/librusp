#include "threadutil.h"

/* THREAD */

pthread_t createThread(void *(*threadFunc) (void *), void *arg, const uint8_t mode) {
	pthread_t tid;
	pthread_attr_t attr;

	if (pthread_attr_init(&attr) != 0) 
		ERREXIT("Cannot initialize thread attributes.");
	
	if (mode == THREAD_DETACHED) {

		if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) 
			ERREXIT("Cannot set thread attribute.");
	}

	if (pthread_create(&tid, &attr, threadFunc, arg) != 0) 
		ERREXIT("Cannot create thread.");

	if (pthread_attr_destroy(&attr) != 0) 
		ERREXIT("Cannot destroy thread attributes.");

	return tid;
}

void cancelThread(pthread_t tid) {
	
	if (pthread_cancel(tid) != 0) 
		ERREXIT("Cannot cancel thread.");
}

void *joinThread(pthread_t tid) {
	void *retval;

	if (pthread_join(tid, &retval) != 0) 
		ERREXIT("Cannot join thread.");

	return retval;
}

/* MUTEX */

void initializeMutex(pthread_mutex_t *mtx) {

	if (pthread_mutex_init(mtx, NULL) != 0) 
		ERREXIT("Cannot initialize mutex.");
}

void destroyMutex(pthread_mutex_t *mtx) {

	if (pthread_mutex_destroy(mtx) != 0)
		ERREXIT("Cannot destroy mutex.");

	free(mtx);
}

void lockMutex(pthread_mutex_t *mtx) {

	if (pthread_mutex_lock(mtx) != 0) 
		ERREXIT("Cannot lock mutex.");
}

void unlockMutex(pthread_mutex_t *mtx) {

	if (pthread_mutex_unlock(mtx) != 0) 
		ERREXIT("Cannot unlock mutex.");
}

/* CONDITION VARIABLE */

void initializeConditionVariable(pthread_cond_t *cnd) {
	
	if (pthread_cond_init(cnd, NULL) != 0) 
		ERREXIT("Cannot initialize condition variable.");
}

void destroyConditionVariable(pthread_cond_t *cnd) {

	if (pthread_cond_destroy(cnd) != 0) 
		ERREXIT("Cannot destroy condition variable.");

	free(cnd);
}

void waitConditionVariable(pthread_cond_t *cnd, pthread_mutex_t *mtx) {

	if (pthread_cond_wait(cnd, mtx) != 0)
		ERREXIT("Cannot wait for condition variable");	
}

void signalConditionVariable(pthread_cond_t *cnd) {
	
	if (pthread_cond_signal(cnd) != 0) 
		ERREXIT("Cannot signal condition variable.");
}

void broadcastConditionVariable(pthread_cond_t *cnd) {

	if (pthread_cond_broadcast(cnd) != 0) 
		ERREXIT("Cannot broadcast condition variable.");
}
