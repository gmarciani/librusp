#include <stdlib.h>
#include <stdio.h>
#include "../util/timerutil.h"

static void timeout(int sig) {
	printf("TIMEOUT!\n");
}

int main(void) {

	startTimeout(2000, 5000);

	registerTimeoutHandler(timeout);

	while (1) {

	}	
	
	exit(EXIT_SUCCESS);
}
