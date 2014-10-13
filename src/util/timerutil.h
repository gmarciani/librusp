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

#ifndef ERREXIT
#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)
#endif

timer_t createTimer(void (*handler) (union sigval), void *arg);

void freeTimer(const timer_t timerid);

void setTimer(const timer_t timerid, const long double value, const long double ivalue);

long double getElapsed(const struct timespec start, const struct timespec end);

struct timespec getTimespec(const long double value);

#endif /* TIMERUTIL_H_ */
