#include <stdlib.h>
#include <stdio.h>
#include "../core/rudpsegment.h"

#define ISN		13
#define WNDSIZE 3

int main(void) {
	Stream stream;
	SegmentList list;
	char *input = NULL;
	char *strlist = NULL;
	char *strsgm = NULL;
	int i;

	list = createSegmentList(ISN, WNDSIZE);

	input = getUserInput("[TO SEND]>");

	stream = createStream(input);

	for (i = 0; i < stream.size; i++)
		submitSegment(&list, stream.segments[i]);

	strlist = listToString(list);
	printf("%s\nList size: %lu Max Window Size: %lu Actual Window Size: %lu\n", strlist, list.size, list.wndsize, list.awndsize);

	free(strlist);
	free(input);

	while (1) {
		input = getUserInput("[ACK]>");
		if (strcmp(input, "quit") == 0) {
			free(input);			
			break;
		}			
		submitAck(&list, strtoul(input, NULL, 10));
		strlist = listToString(list);
		printf("%s\nList size: %lu Max Window Size: %lu Actual Window Size: %lu\n", strlist, list.size, list.wndsize, list.awndsize);	
		Element *curr = list.wndbase;
		while (curr) {
			if (list.wndend)
				if (curr == list.wndend->next)
					break;
			if (curr->status == _RUDP_UNACKED) {
				strsgm = segmentToString(*(curr->segment));
				printf("To retransmit: %s\n", strsgm);
				free(strsgm);
			}
			curr = curr->next;
		}
		free(strlist);
		free(input);
	}	

	freeSegmentList(&list);
	freeStream(&stream);

	exit(EXIT_SUCCESS);
}
