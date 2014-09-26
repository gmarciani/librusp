#include <stdlib.h>
#include <stdio.h>
#include "../core/rudpsegment.h"

int main(void) {
	Stream stream;
	char *input, *strsgm = NULL;
	int i;

	input = getUserInput("[DATA]>");

	stream = createStream(input);

	for (i = 0; i < stream.size; i++) {
		strsgm = segmentToString(stream.segments[i]);
		printf("%s\n", strsgm);
		free(strsgm);
	}

	free(input);

	freeStream(&stream);	
	
	exit(EXIT_SUCCESS);
}
