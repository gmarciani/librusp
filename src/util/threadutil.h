#ifndef THREADUTIL_H_
#define THREADUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#define THREAD_DETACHED 0
#define THREAD_JOINABLE 1

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

/* THREAD */

pthread_t createThread(void *(*threadFunc) (void *), void *arg, const uint8_t mode);

void cancelThread(pthread_t tid);

void *joinThread(pthread_t tid);

/* MUTEX */

void initializeMutex(pthread_mutex_t *mtx);

void destroyMutex(pthread_mutex_t *mtx);

void lockMutex(pthread_mutex_t *mtx);

void unlockMutex(pthread_mutex_t *mtx);

/* CONDITION VARIABLE */

void initializeConditionVariable(pthread_cond_t *cnd);

void destroyConditionVariable(pthread_cond_t *cnd);

void waitConditionVariable(pthread_cond_t *cnd, pthread_mutex_t *mtx);

void signalConditionVariable(pthread_cond_t *cnd);

void broadcastConditionVariable(pthread_cond_t *cnd);

#endif /* THREADUTIL_H_ */
