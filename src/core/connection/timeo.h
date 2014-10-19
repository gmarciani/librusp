#ifndef TIMEO_H_
#define TIMEO_H_

#include <stdlib.h>
#include <stdio.h>
#include "../../util/timeutil.h"
#include "../../util/threadutil.h"
#include "../../util/macroutil.h"

// EXTIMATED RTT
#define RUDP_EXTRTT_A (long double) 0.875
#define RUDP_EXTRTT_B (long double) 0.125
#define RUDP_EXTRTT(extRTT, sampleRTT) (long double) ((RUDP_EXTRTT_A * (extRTT)) + (RUDP_EXTRTT_B * (sampleRTT)))

// DEVIATION RTT
#define RUDP_DEVRTT_A (long double) 0.75
#define RUDP_DEVRTT_B (long double) 0.25
#define RUDP_DEVRTT(devRTT, extRTT, sampleRTT) (long double) ((RUDP_DEVRTT_A * (devRTT)) + RUDP_DEVRTT_B * (labs((extRTT) - (sampleRTT))))

// TIMEOUT
#define RUDP_TIMEO_A 1
#define RUDP_TIMEO_B 4
#define RUDP_TIMEO(extRTT, devRTT) (long double) (RUDP_TIMEO_A * (extRTT) + RUDP_TIMEO_B * (devRTT))
#define RUDP_TIMEOUP 1.50
#define RUDP_TIMEODW 0.95

typedef struct Timeout {
	long double extRTT;
	long double devRTT;
	long double value;
	timer_t timer;

	pthread_rwlock_t *rwlock;
} Timeout;

Timeout *createTimeout(long double sampleRTT, void (*handler) (union sigval), void *arg);

void freeTimeout(Timeout *timeout);

short isTimeoutDisarmed(Timeout *timeout);

long double getTimeoutValue(Timeout *timeout);

void updateTimeout(Timeout *timeout, const long double sampleRTT);

void startTimeout(Timeout *timeout);

void stopTimeout(Timeout *timeout);

#endif /* TIMEO_H_ */
