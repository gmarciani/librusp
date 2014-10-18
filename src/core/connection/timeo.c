#include "timeo.h"

Timeout *createTimeout(long double sampleRTT, void (*handler) (union sigval), void *arg) {
	Timeout *timeout = NULL;

	if (!(timeout = malloc(sizeof(Timeout))))
		ERREXIT("Cannot allocate memory for timeout");

	timeout->extRTT = sampleRTT;

	timeout->devRTT = 0.0;

	timeout->value = sampleRTT;

	timeout->timer = createTimer(handler, arg);

	timeout->mtx = createMutex();

	return timeout;
}

void freeTimeout(Timeout *timeout) {

	freeTimer(timeout->timer);

	destroyMutex(timeout->mtx);

	free(timeout);
}

short isTimeoutDisarmed(Timeout *timeout) {
	return isTimerDisarmed(timeout->timer);
}

long double getTimeoutValue(Timeout *timeout) {
	long double value;

	lockMutex(timeout->mtx);

	value = timeout->value;

	unlockMutex(timeout->mtx);

	return value;
}

void updateTimeout(Timeout *timeout, const long double sampleRTT) {
	lockMutex(timeout->mtx);

	timeout->extRTT = RUDP_EXTRTT(timeout->extRTT, sampleRTT);

	timeout->devRTT = RUDP_DEVRTT(timeout->devRTT, timeout->extRTT, sampleRTT);

	timeout->value = RUDP_TIMEO(timeout->extRTT, timeout->devRTT);

	unlockMutex(timeout->mtx);
}

void startTimeout(Timeout *timeout) {
	lockMutex(timeout->mtx);

	setTimer(timeout->timer, timeout->value, 0.0);

	unlockMutex(timeout->mtx);
}

void stopTimeout(Timeout *timeout) {
	lockMutex(timeout->mtx);

	setTimer(timeout->timer, 0.0, 0.0);

	unlockMutex(timeout->mtx);
}
