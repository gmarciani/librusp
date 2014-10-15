#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
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

	printf("# Converting from double to timespec: value %LF\n", NTIME);

	ts = getTimespec(NTIME);

	printf("sec:%ld nsec:%ld\n", ts.tv_sec, ts.tv_nsec);

	if (ts.tv_sec != 0 || ts.tv_nsec != 0)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	printf("# Converting from double to timespec: value %LF\n", LTIME);

	ts = getTimespec(LTIME);

	printf("sec:%ld nsec:%ld\n", ts.tv_sec, ts.tv_nsec);

	if (ts.tv_sec != 0 || ts.tv_nsec != 500000000)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	printf("# Converting from double to timespec: value %LF\n", MTIME);

	ts = getTimespec(MTIME);

	printf("sec:%ld nsec:%ld\n", ts.tv_sec, ts.tv_nsec);

	if (ts.tv_sec != 1 || ts.tv_nsec != 500000000)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	printf("# Converting from double to timespec: value %LF\n", HTIME);

	ts = getTimespec(HTIME);

	printf("sec:%ld nsec:%ld\n", ts.tv_sec, ts.tv_nsec);

	if (ts.tv_sec != 3 || ts.tv_nsec != 500000000)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	
}

static void testTimevalConversion(void) {
	struct timeval tv;

	printf("# Converting from double to timeval: value %LF\n", NTIME);

	tv = getTimeval(NTIME);

	printf("sec:%ld usec:%ld\n", tv.tv_sec, tv.tv_usec);

	if (tv.tv_sec != 0 || tv.tv_usec != 0)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	printf("# Converting from double to timeval: value %LF\n", LTIME);

	tv = getTimeval(LTIME);

	printf("sec:%ld usec:%ld\n", tv.tv_sec, tv.tv_usec);

	if (tv.tv_sec != 0 || tv.tv_usec != 500000)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	printf("# Converting from double to timeval: value %LF\n", MTIME);

	tv = getTimeval(MTIME);

	printf("sec:%ld usec:%ld\n", tv.tv_sec, tv.tv_usec);

	if (tv.tv_sec != 1 || tv.tv_usec != 500000)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	printf("# Converting from double to timeval: value %LF\n", HTIME);

	tv = getTimeval(HTIME);

	printf("sec:%ld usec:%ld\n", tv.tv_sec, tv.tv_usec);

	if (tv.tv_sec != 3 || tv.tv_usec != 500000)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");
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
