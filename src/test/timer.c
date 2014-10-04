#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../util/timerutil.h"

static uint64_t tvnsec = 2000000000; // 2 seconds

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t cnd = PTHREAD_COND_INITIALIZER;

static void timeoutFunc(union sigval value);

int main(void) {
	timer_t timerid;
	int count = 0;

	printf("# Creating timer #\n");

	timerid = createTimer(timeoutFunc, &count);

	printf("# Setting periodic timer expiration: %llu nanoseconds #\n", (long long) tvnsec);

	setTimer(timerid, tvnsec, TIMER_PERIODIC);

	pthread_mutex_lock(&mtx);

	while(count < 5)
		pthread_cond_wait(&cnd, &mtx);

	printf("# Disarming timer #\n");

	setTimer(timerid, 0, 0);

	pthread_mutex_unlock(&mtx);	

	printf("# Freeing timer #\n");

	freeTimer(timerid);
	
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
