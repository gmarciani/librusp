#include "timeo.h"

Timeout *createTimeout(long double sampleRTT) {
	Timeout *timeout = NULL;

	if (!(timeout = malloc(sizeof(Timeout))))
		ERREXIT("Cannot allocate memory for timeout");

	timeout->extRTT = sampleRTT;

	timeout->devRTT = 0.0;

	timeout->value = sampleRTT;

	timeout->rwlock = createRWLock();

	return timeout;
}

void freeTimeout(Timeout *timeout) {

	freeRWLock(timeout->rwlock);

	free(timeout);
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
