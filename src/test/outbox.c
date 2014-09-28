#include <stdlib.h>
#include <stdio.h>
#include "../core/rudpoutbox.h"
#include "../core/rudpsegment.h"
#include "../util/stringutil.h"

#define ISN		13
#define WNDSIZE 3

int main(void) {
	Stream stream;
	SegmentOutbox outbox;
	char *input = NULL;
	char *stroutbox = NULL;
	char *strsgm = NULL;
	int i;

	outbox = createOutbox(ISN, WNDSIZE);

	input = getUserInput("[TO SEND]>");

	stream = createStream(input);

	for (i = 0; i < stream.size; i++)
		submitSegmentToOutbox(&outbox, stream.segments[i]);

	stroutbox = outboxToString(outbox);
	printf("%s\nOutbox size: %lu Max Window Size: %lu Actual Window Size: %lu\n", stroutbox, outbox.size, outbox.wndsize, outbox.awndsize);

	free(stroutbox);
	free(input);

	while (1) {
		input = getUserInput("[ACK]>");
		if (strcmp(input, "quit") == 0) {
			free(input);			
			break;
		}			
		submitAckToOutbox(&outbox, strtoul(input, NULL, 10));
		stroutbox = outboxToString(outbox);
		printf("%s\nOutbox size: %lu Max Window Size: %lu Actual Window Size: %lu\n", stroutbox, outbox.size, outbox.wndsize, outbox.awndsize);	
		OutboxElement *curr = outbox.wndbase;
		while (curr) {
			if (outbox.wndend)
				if (curr == outbox.wndend->next)
					break;
			if (curr->status == _RUDP_UNACKED) {
				strsgm = segmentToString(*(curr->segment));
				printf("To retransmit: %s\n", strsgm);
				free(strsgm);
			}
			curr = curr->next;
		}
		free(stroutbox);
		free(input);
	}	

	freeOutbox(&outbox);
	freeStream(&stream);

	exit(EXIT_SUCCESS);
}
