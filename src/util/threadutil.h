#ifndef THREADUTIL_H_
#define THREADUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include "macroutil.h"

#define THREAD_DETACHED 0
#define THREAD_JOINABLE 1

/* THREAD */

pthread_t createThread(void *(*threadFunc) (void *), void *arg, const uint8_t mode);

void cancelThread(pthread_t tid);

void *joinThread(pthread_t tid);

/* MUTEX */

pthread_mutex_t *createMutex();

void freeMutex(pthread_mutex_t *mtx);

void lockMutex(pthread_mutex_t *mtx);

void unlockMutex(pthread_mutex_t *mtx);

/* CONDITION VARIABLE */

pthread_cond_t *createConditionVariable();

void freeConditionVariable(pthread_cond_t *cnd);

void waitConditionVariable(pthread_cond_t *cnd, pthread_mutex_t *mtx);

void waitTimeoutConditionVariable(pthread_cond_t *cnd, pthread_mutex_t *mtx, const struct timespec timeout);

void signalConditionVariable(pthread_cond_t *cnd);

void broadcastConditionVariable(pthread_cond_t *cnd);

/* READ/WRITE LOCK */

pthread_rwlock_t *createRWLock(void);

void freeRWLock(pthread_rwlock_t *rwlock);

void lockRead(pthread_rwlock_t *rwlock);

void lockWrite(pthread_rwlock_t *rwlock);

void unlockRWLock(pthread_rwlock_t *rwlock);

/* SPINLOCK */


/* SEMAPHORE */

sem_t *createSemaphore(const unsigned int value);

void freeSemaphore(sem_t *sem);

void incrementSemaphore(sem_t *sem);

void decrementSemaphore(sem_t *sem);

#endif /* THREADUTIL_H_ */
