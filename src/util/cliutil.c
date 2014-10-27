#include "cliutil.h"

static char PROG_TRUE[20] = "====================";
static char PROG_FALSE[20] = "                    ";

void progressBar(const long long i, const long long n) {
	int ratio;
	int curr;

    ratio = i * 100 / n;

    if (ratio % 5 != 0)
    	return;

    printf("\r");

    fflush(stdout);

    curr = ratio / 5;

    printf("%3d%% [%.*s%.*s]", ratio, curr, PROG_TRUE, 20-curr, PROG_FALSE);
}

void progressCounter(const long long count) {
	printf("\r");

	fflush(stdout);

	printf("[%lld]", count);
}
