#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../util/timerutil.h"

#define TIMEOLONG (uint64_t) 2000000000
#define TIMEONOW (uint64_t) 1000

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t cnd = PTHREAD_COND_INITIALIZER;

static void timeoutFunc(union sigval value);

int main(void) {
	timer_t timerid;
	int count = 0;

	printf("# Creating timer\n");

	timerid = createTimer(timeoutFunc, &count);

	printf("Timer created\n");

	printf("# Setting timer once expiration: (only) first timeout: %lu nanos\n", TIMEOLONG);

	setTimer(timerid, TIMEOLONG, 0);

	pthread_mutex_lock(&mtx);

	while(count < 1)
		pthread_cond_wait(&cnd, &mtx);

	pthread_mutex_unlock(&mtx);	

	printf("# Setting timer periodic expiration: first timeout: %lu nanos periodic timeout: %lu \n", TIMEONOW, TIMEOLONG);

	setTimer(timerid, TIMEONOW, TIMEOLONG);

	pthread_mutex_lock(&mtx);

	while(count < 5)
		pthread_cond_wait(&cnd, &mtx);

	pthread_mutex_unlock(&mtx);	

	printf("# Setting timer periodic expiration: periodic timeout: %lu \n", TIMEOLONG);

	setTimer(timerid, TIMEOLONG, TIMEOLONG);

	pthread_mutex_lock(&mtx);

	while(count < 10)
		pthread_cond_wait(&cnd, &mtx);

	pthread_mutex_unlock(&mtx);	

	printf("# Disarming timer\n");

	setTimer(timerid, 0, 0);

	printf("Timer disarmed\n");

	printf("# Freeing timer\n");

	freeTimer(timerid);

	printf("Timer freed\n");
	
	exit(EXIT_SUCCESS);
}

static void timeoutFunc(union sigval value) {
	int *count = (int *) value.sival_ptr;

	printf("TIMEOUT!\n");

	pthread_mutex_lock(&mtx);

	*(count) += 1;

	printf("Value: %d\n", *count);

	pthread_mutex_unlock(&mtx);

	pthread_cond_signal(&cnd);		
}
