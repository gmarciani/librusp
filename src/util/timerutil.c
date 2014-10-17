#include "timerutil.h"

timer_t createTimer(void (*handler) (union sigval), void *arg) {
	timer_t timerid;
	struct sigevent event;

	event.sigev_notify = SIGEV_THREAD;

	event.sigev_notify_function = handler;

	event.sigev_notify_attributes = NULL;

	event.sigev_value.sival_ptr = arg;	

	errno = 0;

	if (timer_create(CLOCK_REALTIME, &event, &timerid) != 0) 
		ERREXIT("Cannot create timer: %s", strerror(errno));

	return timerid;
}

void freeTimer(const timer_t timerid) {
	errno = 0;

	if (timer_delete(timerid) != 0)
		ERREXIT("Cannot delete timer: %s", strerror(errno));
}

void setTimer(const timer_t timerid, const long double value, const long double ivalue) {
	struct itimerspec timeout;

	timeout.it_value = getTimespec(value);

	timeout.it_interval = getTimespec(ivalue);

	errno = 0;

	if (timer_settime(timerid, 0, &timeout, NULL) != 0)
		ERREXIT("Cannot set timer: %s", strerror(errno));
		
}

struct itimerspec getTimer(const timer_t timerid) {
	struct itimerspec timeout;

	errno = 0;

	if (timer_gettime(timerid, &timeout) != 0)
		ERREXIT("Cannot set timer: %s", strerror(errno));

	return timeout;
}

short isTimerDisarmed(const timer_t timerid) {
	struct itimerspec timeout;

	timeout = getTimer(timerid);

	return (timeout.it_value.tv_sec == 0 && timeout.it_value.tv_nsec == 0 &&
			timeout.it_interval.tv_sec == 0 && timeout.it_interval.tv_nsec == 0);
}

long double getElapsed(const struct timespec start, const struct timespec end) {
	long double elapsed;

	elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

	return elapsed;
}

struct timespec getTimespec(const long double value) {
	struct timespec time;

	time.tv_sec = (time_t) (value < 1)?0:ceill(value - 1);

	time.tv_nsec = (long) (value * 1000000000) % 1000000000;

	return time;

}
struct timeval getTimeval(const long double value) {
	struct timeval time;

	time.tv_sec = (time_t) (value < 1)?0:ceill(value - 1);

  	time.tv_usec = (suseconds_t) (value * 1000000) % 1000000;

	return time;
}
