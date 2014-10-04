#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "../core/rudpoutbox.h"
#include "../core/rudpsegment.h"
#include "../util/stringutil.h"

#define ISN	4294967280
#define WNDSIZE 3

int main(void) {
	Stream *stream = NULL;
	Outbox *outbox = NULL;
	Segment *retrans = NULL;
	uint32_t ackn;
	uint32_t retransno;
	char *input = NULL;
	char *stroutbox = NULL;
	char *strsgm = NULL;
	int i;

	printf("# Creating Outbox with isn=%u wnds=%u #\n", (uint32_t) ISN, (uint32_t) WNDSIZE);

	outbox = createOutbox(ISN, WNDSIZE);

	puts("# Outbox to string #");

	stroutbox = outboxToString(outbox);

	printf("%s\n", stroutbox);

	free(stroutbox);

	while (1) {

		puts("# Submitting your input message to outbox (type 'quit' to exit test) #");

		input = getUserInput("[TO SEND]>");

		if (strcmp(input, "quit") == 0) {		
			free(input);
			break;
		}

		printf("# Creating stream of segments (MAX_PLD:%d) #\n", RUDP_PLDS);

		stream = createStream(input);

		puts("# Submitting stream of segments to outbox #");

		for (i = 0; i < stream->size; i++) {

			submitSegmentToOutbox(outbox, stream->segments[i]);

			puts("# Submitting segment to outbox #");

			strsgm = segmentToString(stream->segments[i]);

			printf("%s\n", strsgm);

			free(strsgm);

			puts("# Outbox to string #");

			stroutbox = outboxToString(outbox);

			printf("%s\n", stroutbox);

			free(stroutbox);
		}

		freeStream(stream);

		puts("# Outbox to string #");

		stroutbox = outboxToString(outbox);

		printf("%s\n", stroutbox);

		free(stroutbox);

		free(input);

		while (1) {

			puts("# Submitting you ACK to outbox (quit stop submitting ACKs) #");

			input = getUserInput("[ACK]>");

			if (strcmp(input, "quit") == 0) {

				free(input);			

				break;
			}	

			ackn = (uint32_t) strtoul(input, NULL, 10);

			free(input);

			printf("# Submitting ACK %u to outbox #\n", ackn);
		
			submitAckToOutbox(outbox, ackn);

			puts("# Outbox to string #");

			stroutbox = outboxToString(outbox);

			printf("%s\n", stroutbox);

			free(stroutbox);

			puts("# Retrieving segment to retransmit #");

			retrans = getRetransmittableSegments(outbox, &retransno);

			printf("There are %u segments to retransmit\n", retransno);

			for (i = 0; i < retransno; i++) {
				strsgm = segmentToString(retrans[i]);

				printf("%s\n", strsgm);

				free(strsgm);
			}

			free(retrans);

			puts("");
		}

		puts("");
	}			

	puts("# Freeing outbox #");

	freeOutbox(outbox);	

	exit(EXIT_SUCCESS);
}
