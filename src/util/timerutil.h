#ifndef TIMERUTIL_H_
#define TIMERUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <math.h>

#define TIMER_ONCE		0
#define TIMER_PERIODIC 	1	

timer_t createTimer(void (*handler) (union sigval), void *arg);

void freeTimer(const timer_t timerid);

void setTimer(const timer_t timerid, const uint64_t nanos, const uint8_t mode);

#endif /* TIMERUTIL_H_ */
