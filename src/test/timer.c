#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../util/timerutil.h"

#define TIMENONE (long double) 0.0
#define TIMELONG (long double) 2.5
#define TIMESHORT (long double) 0.5

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t cnd = PTHREAD_COND_INITIALIZER;

static void timeoutFunc(union sigval value);

int main(void) {
	timer_t timerid;
	struct timespec start, end;
	long double elapsed;
	int count = 0;

	clock_gettime(CLOCK_MONOTONIC, &start);

	printf("# Creating timer\n");

	timerid = createTimer(timeoutFunc, &count);

	printf("Timer created\n");

	printf("# Setting timer once expiration: (only) first timeout: %LF secs \n", TIMELONG);

	setTimer(timerid, TIMELONG, TIMENONE);

	pthread_mutex_lock(&mtx);

	while(count < 1)
		pthread_cond_wait(&cnd, &mtx);

	pthread_mutex_unlock(&mtx);	

	printf("# Setting timer periodic expiration: first timeout: %LF secs, periodic timeout: %LF secs\n", TIMELONG, TIMESHORT);

	setTimer(timerid, TIMELONG, TIMESHORT);

	pthread_mutex_lock(&mtx);

	while(count < 5)
		pthread_cond_wait(&cnd, &mtx);

	pthread_mutex_unlock(&mtx);	

	printf("# Setting timer periodic expiration: periodic timeout: %LF secs\n", TIMELONG);

	setTimer(timerid, TIMELONG, TIMELONG);

	pthread_mutex_lock(&mtx);

	while(count < 10)
		pthread_cond_wait(&cnd, &mtx);

	pthread_mutex_unlock(&mtx);	

	printf("# Disarming timer\n");

	setTimer(timerid, TIMENONE, TIMENONE);

	printf("Timer disarmed\n");

	printf("# Freeing timer\n");

	freeTimer(timerid);

	printf("Timer freed\n");

	clock_gettime(CLOCK_MONOTONIC, &end);

	elapsed = getElapsed(start, end);

	printf("# Getting total elapsed time\n");

	printf("%LF secs\n", elapsed);
	
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
