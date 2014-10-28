#include "timeo.h"

//static int DBGFILE;

void initializeTimeout(Timeout *timeout, const long double sampleRTT) {
	//char LOG[300];

	pthread_rwlock_init(&(timeout->rwlock), NULL);

	timeout->extRTT = sampleRTT;

	timeout->devRTT = 0.0;

	timeout->value = sampleRTT;

	//DBGFILE = open("DBGFILE_TIMEOUT", O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);

	//sprintf(LOG, "%LF\t%LF\t%LF\n", timeout->extRTT, timeout->devRTT, timeout->value);
	//write(DBGFILE, LOG, strlen(LOG));
}

void destroyTimeout(Timeout *timeout) {
	if (pthread_rwlock_destroy(&(timeout->rwlock)) > 0)
		ERREXIT("Cannot destroy timeout read-write lock.");

	//close(DBGFILE);
}

long double getTimeoutValue(Timeout *timeout) {
	long double value;

	if (pthread_rwlock_rdlock(&(timeout->rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	value = timeout->value;

	if (pthread_rwlock_unlock(&(timeout->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return value;
}

void updateTimeout(Timeout *timeout, const long double sampleRTT) {
	//char LOG[300];
	if (pthread_rwlock_wrlock(&(timeout->rwlock)) > 0)
		ERREXIT("Cannot acquire write-lock.");

	timeout->extRTT = RUDP_EXTRTT_A * timeout->extRTT + RUDP_EXTRTT_B * sampleRTT;

	timeout->devRTT = RUDP_DEVRTT_A * timeout->devRTT + RUDP_DEVRTT_B * fabsl(timeout->extRTT - sampleRTT);

	timeout->value = RUDP_TIMEO_A * timeout->extRTT + RUDP_TIMEO_B * timeout->devRTT;

	//sprintf(LOG, "%LF\t%LF\t%LF\n", timeout->extRTT, timeout->devRTT, timeout->value);
	//write(DBGFILE, LOG, strlen(LOG));

	if (pthread_rwlock_unlock(&(timeout->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");
}
