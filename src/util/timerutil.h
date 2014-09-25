#ifndef TIMERUTIL_H_
#define TIMERUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <math.h>

unsigned long startTimeout(unsigned long millis, unsigned long period);

unsigned long getTimeout();

void registerTimeoutHandler(void (*handler) (int));

#endif /* TIMERUTIL_H_ */
