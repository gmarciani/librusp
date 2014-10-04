#include "threadutil.h"

/* THREAD */

pthread_t createThread(void *(*threadFunc) (void *), void *arg, const uint8_t mode) {
	pthread_t tid;
	pthread_attr_t attr;

	if (pthread_attr_init(&attr) != 0) {
	
		fprintf(stderr, "Cannot initialize thread attributes.\n");

		exit(EXIT_FAILURE);
	}
	
	if (mode == THREAD_DETACHED) {

		if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
			
			fprintf(stderr, "Cannot set thread attribute.\n");

			exit(EXIT_FAILURE);
		}
	}

	if (pthread_create(&tid, &attr, threadFunc, arg) != 0) {

		fprintf(stderr, "Cannot create thread.\n");

		exit(EXIT_FAILURE);
	}

	if (pthread_attr_destroy(&attr) != 0) {

		fprintf(stderr, "Cannot destroy thread attributes.\n");

		exit(EXIT_FAILURE);
	}

	return tid;
}

void cancelThread(pthread_t tid) {
	
	if (pthread_cancel(tid) != 0) {
		
		fprintf(stderr, "Cannot cancel thread.\n");

		exit(EXIT_FAILURE);
	}
}

void *joinThread(pthread_t tid) {
	void *retval;

	if (pthread_join(tid, &retval) != 0) {

		fprintf(stderr, "Cannot join thread.\n");

		exit(EXIT_FAILURE);
	}

	return retval;
}

/* MUTEX */

void initializeMutex(pthread_mutex_t *mtx) {

	if (pthread_mutex_init(mtx, NULL) != 0) {

		fprintf(stderr, "Cannot initialize mutex.\n");

		exit(EXIT_FAILURE);
	}

}

void destroyMutex(pthread_mutex_t *mtx) {

	if (pthread_mutex_destroy(mtx) != 0) {
		
		fprintf(stderr, "Cannot destroy mutex.\n");

		exit(EXIT_FAILURE);
	}

	free(mtx);
}

void lockMutex(pthread_mutex_t *mtx) {

	if (pthread_mutex_lock(mtx) != 0) {

		fprintf(stderr, "Cannot lock mutex.\n");

		exit(EXIT_FAILURE);
	}
}

void unlockMutex(pthread_mutex_t *mtx) {

	if (pthread_mutex_unlock(mtx) != 0) {

		fprintf(stderr, "Cannot unlock mutex.\n");

		exit(EXIT_FAILURE);
	}
}

/* CONDITION VARIABLE */

void initializeConditionVariable(pthread_cond_t *cnd) {
	
	if (pthread_cond_init(cnd, NULL) != 0) {

		fprintf(stderr, "Cannot initialize condition variable.\n");

		exit(EXIT_FAILURE);
	}
}

void destroyConditionVariable(pthread_cond_t *cnd) {

	if (pthread_cond_destroy(cnd) != 0) {
		
		fprintf(stderr, "Cannot destroy condition variable.\n");

		exit(EXIT_FAILURE);
	}

	free(cnd);
}

void signalConditionVariable(pthread_cond_t *cnd) {
	
	if (pthread_cond_signal(cnd) != 0) {

		fprintf(stderr, "Cannot signal condition variable.\n");

		exit(EXIT_FAILURE);
	}
}
