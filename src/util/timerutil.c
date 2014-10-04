#include "timerutil.h"

timer_t createTimer(void (*handler) (union sigval), void *arg) {
	timer_t timerid;
	struct sigevent event;

	event.sigev_notify = SIGEV_THREAD;

	event.sigev_notify_function = handler;

	event.sigev_notify_attributes = NULL;

	event.sigev_value.sival_ptr = arg;	

	if (timer_create(CLOCK_REALTIME, &event, &timerid) != 0) {

		fprintf(stderr, "Cannot create timer.\n");

		exit(EXIT_FAILURE);
	}

	return timerid;
}

void freeTimer(const timer_t timerid) {
	if (timer_delete(timerid) != 0) {

		fprintf(stderr, "Cannot delete timer.\n");

		exit(EXIT_FAILURE);
	}
}

void setTimer(const timer_t timerid, const uint64_t nanos, const uint8_t mode) {
	struct itimerspec timerspec;

	timerspec.it_value.tv_sec = (time_t) ceil(nanos / 1000000000);

	timerspec.it_value.tv_nsec = (long) (nanos % 1000000000);

	if (mode == TIMER_ONCE) {
		
		timerspec.it_interval.tv_sec = (time_t) 0;
	
		timerspec.it_interval.tv_nsec = (long) 0;

	} else if (mode == TIMER_PERIODIC) {

		timerspec.it_interval.tv_sec = timerspec.it_value.tv_sec;
	
		timerspec.it_interval.tv_nsec = timerspec.it_value.tv_nsec;
		
	} else {

		fprintf(stderr, "Cannot recognize timer mode.\n");

		exit(EXIT_FAILURE);
	}	

	if (timer_settime(timerid, 0, (const struct itimerspec *) &timerspec, NULL) != 0) {

		fprintf(stderr, "Cannot set timer.\n");

		exit(EXIT_FAILURE);
	}
}
