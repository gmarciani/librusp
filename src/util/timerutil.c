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

void setTimer(const timer_t timerid, const long double value, const long double ivalue) {
	struct itimerspec timeout;

	timeout.it_value = getTimespec(value);

	timeout.it_interval = getTimespec(ivalue);

	if (timer_settime(timerid, 0, &timeout, NULL) != 0)
		ERREXIT("Cannot set timer.");
}

long double getElapsed(const struct timespec start, const struct timespec end) {
	long double elapsed;

	elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

	return elapsed;
}

struct timespec getTimespec(const long double value) {
	struct timespec time;

	time.tv_sec = (time_t) value;

	time.tv_nsec = (long) (value * 1000000000) % 1000000000;

	return time;

}
