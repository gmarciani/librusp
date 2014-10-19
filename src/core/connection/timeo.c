#include "timeo.h"

Timeout *createTimeout(long double sampleRTT, void (*handler) (union sigval), void *arg) {
	Timeout *timeout = NULL;

	if (!(timeout = malloc(sizeof(Timeout))))
		ERREXIT("Cannot allocate memory for timeout");

	timeout->extRTT = sampleRTT;

	timeout->devRTT = 0.0;

	timeout->value = sampleRTT;

	timeout->timer = createTimer(handler, arg);

	timeout->rwlock = createRWLock();

	return timeout;
}

void freeTimeout(Timeout *timeout) {

	freeTimer(timeout->timer);

	freeRWLock(timeout->rwlock);

	free(timeout);
}

short isTimeoutDisarmed(Timeout *timeout) {
	short isdisarmed;

	lockRead(timeout->rwlock);

	isdisarmed = isTimerDisarmed(timeout->timer);

	unlockRWLock(timeout->rwlock);

	return isdisarmed;
}

long double getTimeoutValue(Timeout *timeout) {
	long double value;

	lockRead(timeout->rwlock);

	value = timeout->value;

	unlockRWLock(timeout->rwlock);

	return value;
}

void updateTimeout(Timeout *timeout, const long double sampleRTT) {
	lockWrite(timeout->rwlock);

	timeout->extRTT = RUDP_EXTRTT(timeout->extRTT, sampleRTT);

	timeout->devRTT = RUDP_DEVRTT(timeout->devRTT, timeout->extRTT, sampleRTT);

	timeout->value = RUDP_TIMEO(timeout->extRTT, timeout->devRTT);

	unlockRWLock(timeout->rwlock);
}

void startTimeout(Timeout *timeout) {
	lockWrite(timeout->rwlock);

	setTimer(timeout->timer, timeout->value, 0.0);

	unlockRWLock(timeout->rwlock);
}

void stopTimeout(Timeout *timeout) {
	lockWrite(timeout->rwlock);

	setTimer(timeout->timer, 0.0, 0.0);

	unlockRWLock(timeout->rwlock);
}
