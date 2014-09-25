#include "../protocol/core/rudpsegment.h"

int main(void) {
	Segment sgm;
	char *ssgm;

	sgm = createSegment(_RUDP_DAT, 51, 23, "Hello world!");

	ssgm = serializeSegment(sgm);

	printf("Serialized: %s\n", ssgm);

	sgm = deserializeSegment(ssgm);

	ssgm = serializeSegment(sgm);

	printf("Serialized: %s\n", ssgm);

	free(ssgm);

	exit(EXIT_SUCCESS);
}
