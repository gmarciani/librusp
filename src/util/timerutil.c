#include "timerutil.h"

timer_t createTimer(void (*handler) (union sigval), void *arg) {
	timer_t timerid;
	struct sigevent event;

	event.sigev_notify = SIGEV_THREAD;

	event.sigev_notify_function = handler;

	event.sigev_notify_attributes = NULL;

	event.sigev_value.sival_ptr = arg;	

	if (timer_create(CLOCK_REALTIME, &event, &timerid) != 0) 
		ERREXIT("Cannot create timer.");

	return timerid;
}

void freeTimer(const timer_t timerid) {

	if (timer_delete(timerid) != 0)
		ERREXIT("Cannot delete timer.");
}

void setTimer(const timer_t timerid, const uint64_t nanos, const uint64_t inanos) {
	struct itimerspec timerspec;

	timerspec.it_value.tv_sec = (time_t) ceil(nanos / 1000000000);

	timerspec.it_value.tv_nsec = (long) (nanos % 1000000000);

	timerspec.it_interval.tv_sec = (time_t) ceil(inanos / 1000000000);

	timerspec.it_interval.tv_nsec = (long) (inanos % 1000000000);

	if (timer_settime(timerid, 0, (const struct itimerspec *) &timerspec, NULL) != 0)
		ERREXIT("Cannot set timer.");
}
