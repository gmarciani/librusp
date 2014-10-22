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

	timeout->extRTT = RUDP_EXTRTT_A * timeout->extRTT + RUDP_EXTRTT_B * sampleRTT;

	timeout->devRTT = RUDP_DEVRTT_A * timeout->devRTT + RUDP_DEVRTT_B * fabsl(timeout->extRTT - sampleRTT);

	timeout->value = RUDP_TIMEO_A * timeout->extRTT + RUDP_TIMEO_B * timeout->devRTT;

	unlockRWLock(timeout->rwlock);
}
