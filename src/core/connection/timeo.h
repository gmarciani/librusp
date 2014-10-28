#ifndef TIMEO_H_
#define TIMEO_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include "../../util/timeutil.h"
#include "../../util/macroutil.h"


#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

// EXTIMATED RTT
#define RUDP_EXTRTT_A 0.875
#define RUDP_EXTRTT_B 0.125

// DEVIATION RTT
#define RUDP_DEVRTT_A 0.75
#define RUDP_DEVRTT_B 0.25

// TIMEOUT
#define RUDP_TIMEO_A 1
#define RUDP_TIMEO_B 4

#define RUDP_TIMEOUP 1.50
#define RUDP_TIMEODW 0.95

typedef struct Timeout {
	long double extRTT;
	long double devRTT;
	long double value;

	pthread_rwlock_t rwlock;
} Timeout;

void initializeTimeout(Timeout *timeout, const long double sampleRTT);

void destroyTimeout(Timeout *timeout);

long double getTimeoutValue(Timeout *timeout);

void updateTimeout(Timeout *timeout, const long double sampleRTT);

#endif /* TIMEO_H_ */
