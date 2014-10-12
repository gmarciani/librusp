#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "../core/rudpoutbox.h"
#include "../util/stringutil.h"

#define WNDB 0
#define WNDS 10
#define TIMEOUT 5000000000

#define MSG "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
			\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
			\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
			\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
			\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
			\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
			\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
			\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
			\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
			\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

int main(void) {
	Outbox *outbox = NULL;
	char *input = NULL;
	uint32_t ackn;

	printf("# Creating Outbox (wndb:%u wnds:%u timeout:%lu)\n", (uint32_t) WNDB, (uint32_t) WNDS, (uint64_t) TIMEOUT);

	outbox = createOutbox(WNDB, WNDS, 1, TIMEOUT);

	printf("Outbox created\n");

	printf("# Submitting user message to outbox (type 'quit' to exit test):\n");

	writeOutboxUserBuffer(outbox, MSG, strlen(MSG));
	
	printf("Message submitted\n");

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

		printf("\n");
	}

	printf("# Freeing outbox\n");

	freeOutbox(outbox);	

	printf("Outbox freed\n");

	exit(EXIT_SUCCESS);
}
