#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../protocol/_rudp.h"

int main(int argc, char **argv) {
	rudpsgm_t sgm;
	char *ssgm;

	sgm = _rudpCreateSegment(INIT, 20, 10, 5, "Ciao mi chiamo Giacomo");

	ssgm = _rudpSerializeSegment(sgm);

	printf("Serialized: %s\n", ssgm);

	sgm = _rudpDeserializeSegment(ssgm);

	ssgm = _rudpSerializeSegment(sgm);

	printf("Serialized: %s\n", ssgm);
	
	exit(EXIT_SUCCESS);
}
