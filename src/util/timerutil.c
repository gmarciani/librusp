#include "timerutil.h"

unsigned long startTimeout(unsigned long millis, unsigned long period) {
	unsigned long oldmillis;
	struct itimerval timeout, oldtimeout;

	timeout.it_value.tv_sec = ceil(millis / 1000);

	timeout.it_value.tv_usec = (millis % 1000) * 1000;

	timeout.it_interval.tv_sec = ceil(period / 1000);

	timeout.it_interval.tv_usec = (period % 1000) * 1000;
	
	if (setitimer(ITIMER_REAL, &timeout, &oldtimeout) == -1) {
		fprintf(stderr, "Cannot set timeout.\n");
		exit(EXIT_FAILURE);
	}

	oldmillis = (oldtimeout.it_value.tv_sec * 1000) + (oldtimeout.it_value.tv_usec / 1000);

	return oldmillis;
} 

unsigned long getTimeout() {
	unsigned long millis;
	struct itimerval timeout;
	

	if (getitimer(ITIMER_REAL, &timeout) == -1) {
		fprintf(stderr, "Cannot get timeout.\n");
		exit(EXIT_FAILURE);
	}

	millis = (timeout.it_value.tv_sec * 1000) + (timeout.it_value.tv_usec / 1000);

	return millis;
}

void registerTimeoutHandler(void (*handler) (int)) {
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);

	sa.sa_flags = 0;

	sa.sa_handler = handler;

	if (sigaction(SIGALRM, &sa, NULL) == -1) {
		fprintf(stderr, "Cannot register handler for timeout.\n");
		exit(EXIT_FAILURE);
	}
}
