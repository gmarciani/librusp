#include <stdlib.h>
#include <stdio.h>
#include "../core/rudpoutbox.h"
#include "../core/rudpsegment.h"
#include "../util/stringutil.h"

#define ISN		13
#define WNDSIZE 3

int main(void) {
	Stream stream;
	SegmentOutbox *outbox;
	uint32_t ackn;
	char *input = NULL;
	char *stroutbox = NULL;
	char *strsgm = NULL;
	int i;

	printf("# Creating Outbox with isn=%u wnds=%u #\n", ISN, WNDSIZE);

	outbox = createOutbox(ISN, WNDSIZE);

	puts("# Outbox to string #");

	stroutbox = outboxToString(outbox);

	printf("%s\n", stroutbox);

	free(stroutbox);
	
	puts("# Submitting your input message to outbox #");

	input = getUserInput("[TO SEND]>");

	puts("# Creating stream of segments #");

	stream = createStream(input);

	puts("# Submitting stream of segments to outbox #");

	for (i = 0; i < stream.size; i++) {

		submitSegmentToOutbox(outbox, stream.segments[i]);

		puts("# Submitting segment to outbox #");

		strsgm = segmentToString(stream.segments[i]);

		printf("%s\n", strsgm);

		free(strsgm);
	}

	freeStream(&stream);

	puts("# Outbox to string #");

	stroutbox = outboxToString(outbox);

	printf("%s\n", stroutbox);

	free(stroutbox);

	free(input);

	while (1) {

		puts("# Submitting you ACK to outbox (quit to exit test) #");

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
	
		OutboxElement *curr = outbox->wndb;

		while (curr) {

			if (outbox->wnde)
				if (curr == outbox->wnde->next)
					break;

			if (curr->status == RUDP_UNACKED) {

				strsgm = segmentToString(*(curr->segment));

				printf("%s\n", strsgm);

				free(strsgm);
			}

			curr = curr->next;
		}
	}		

	puts("# Freeing outbox #");

	freeOutbox(outbox);	

	exit(EXIT_SUCCESS);
}
