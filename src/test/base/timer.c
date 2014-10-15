#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "../../util/timerutil.h"
#include "../../util/macroutil.h"

#define NTIME 0.0L
#define LTIME 0.5L
#define MTIME 1.5L
#define HTIME 3.5L

static timer_t timerid;

static int count;

static void testTimespecConversion(void);

static void testTimevalConversion(void);

static void creation(void);

static void setExpiration(void);

static void disarm(void);

static void deallocation(void);

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t cnd = PTHREAD_COND_INITIALIZER;

static void timeoutFunc(union sigval value);

int main(void) {

	testTimespecConversion();

	testTimevalConversion();

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

static void testTimespecConversion(void) {
	struct timespec ts;

	printf("# Converting from long double to timespec...");

	ts = getTimespec(NTIME);

	assert(ts.tv_sec == 0 || ts.tv_nsec == 0);

	ts = getTimespec(LTIME);

	assert(ts.tv_sec == 0 || ts.tv_nsec == 500000000);

	ts = getTimespec(MTIME);

	assert(ts.tv_sec == 1 || ts.tv_nsec == 500000000);

	ts = getTimespec(HTIME);

	assert(ts.tv_sec == 3 || ts.tv_nsec == 500000000);

	printf("OK\n");
}

static void testTimevalConversion(void) {
	struct timeval tv;

	printf("# Converting from long double to timeval...");

	tv = getTimeval(NTIME);

	assert(tv.tv_sec == 0 || tv.tv_usec == 0);

	tv = getTimeval(LTIME);

	assert(tv.tv_sec == 0 || tv.tv_usec == 500000);

	tv = getTimeval(MTIME);

	assert(tv.tv_sec == 1 || tv.tv_usec == 500000);

	tv = getTimeval(HTIME);

	assert(tv.tv_sec == 3 || tv.tv_usec == 500000);

	printf("OK\n");
}

static void creation(void) {
	printf("# Creating timer...");

	timerid = createTimer(timeoutFunc, &count);

	printf("OK\n");
}

static void setExpiration(void) {
	printf("# Setting timer periodic expiration: %LF secs and periodic %LF secs...", HTIME, LTIME);

	setTimer(timerid, HTIME, LTIME);

	printf("OK\n");
}

static void disarm(void) {
	printf("# Disarming timer...");

	setTimer(timerid, NTIME, NTIME);

	printf("OK\n");
}

static void deallocation(void) {
	printf("# Freeing timer...");

	freeTimer(timerid);

	printf("OK\n");
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
