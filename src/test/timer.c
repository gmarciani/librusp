#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../util/timerutil.h"

#define NTIME 0.0L
#define LTIME 1.5L
#define HTIME 3.5L

#ifndef ERREXIT
#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)
#endif

static timer_t timerid;

static int count;

static void creation(void);

static void setExpiration(void);

static void disarm(void);

static void deallocation(void);

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t cnd = PTHREAD_COND_INITIALIZER;

static void timeoutFunc(union sigval value);

int main(void) {

	creation();

	setExpiration();

	pthread_mutex_lock(&mtx);

	while(count < 10)
		pthread_cond_wait(&cnd, &mtx);

	pthread_mutex_unlock(&mtx);

	disarm();	

	deallocation();
	
	exit(EXIT_SUCCESS);
}

static void creation(void) {
	printf("# Creating timer\n");

	timerid = createTimer(timeoutFunc, &count);

	printf("SUCCESS\n");
}

static void setExpiration(void) {
	printf("# Setting timer periodic expiration: first timeout: %LF secs, periodic timeout: %LF secs\n", HTIME, LTIME);

	setTimer(timerid, HTIME, LTIME);

	printf("SUCCESS\n");
}

static void disarm(void) {
	printf("# Disarming timer\n");

	setTimer(timerid, NTIME, NTIME);

	printf("SUCCESS\n");
}

static void deallocation(void) {
	printf("# Freeing timer\n");

	freeTimer(timerid);

	printf("SUCCESS\n");
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
