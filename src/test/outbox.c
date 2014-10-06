#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "../core/rudpoutbox.h"
#include "../util/stringutil.h"

#define WNDB 0
#define WNDS 5

int main(void) {
	Outbox *outbox = NULL;
	uint32_t ackn;
	char *input = NULL;
	char *stroutbox = NULL;

	printf("# Creating Outbox (wndb=%u wnds=%u)\n", (uint32_t) WNDB, (uint32_t) WNDS);

	outbox = createOutbox(WNDB, WNDS);

	printf("Outbox created\n");

	printf("# Outbox to string:\n");

	stroutbox = outboxToString(outbox);

	printf("%s\n", stroutbox);

	free(stroutbox);

	printf("# Submitting user message to outbox (type 'quit' to exit test):\n");

	input = getUserInput("[DATA]>");

	if (strcmp(input, "quit") == 0) {	

		free(input);

		printf("# Freeing outbox #\n");

		freeOutbox(outbox);	

		exit(EXIT_SUCCESS);
	}

	writeOutboxUserBuffer(outbox, input, strlen(input));

	free(input);

	printf("# Outbox to string:\n");

	stroutbox = outboxToString(outbox);

	printf("%s\n", stroutbox);

	free(stroutbox);

	while (1) {

		printf("# Submitting your ACKs to outbox (type 'quit' to exit test):\n");

		input = getUserInput("[ACK]>");

		if (strcmp(input, "quit") == 0) {

			free(input);			

			break;
		}	

		ackn = (uint32_t) strtoul(input, NULL, 10);

		free(input);

		printf("# Submitting ACK %u to outbox\n", ackn);
	
		submitAckToOutbox(outbox, ackn);

		printf("ACK submitted\n");

		printf("# Outbox to string:\n");

		stroutbox = outboxToString(outbox);

		printf("%s\n", stroutbox);

		free(stroutbox);

		printf("\n");
	}

	printf("# Freeing outbox\n");

	freeOutbox(outbox);	

	printf("Outbox freed\n");

	exit(EXIT_SUCCESS);
}
