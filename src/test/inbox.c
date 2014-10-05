#include <stdlib.h>
#include <stdio.h>
#include "../core/rudpinbox.h"
#include "../core/rudpsegment.h"
#include "../util/stringutil.h"

#define ISN	0
#define WNDSIZE 5

int main(void) {
	Inbox *inbox = NULL;
	Segment sgm;
	uint8_t ctrl;
	uint32_t seqn;
	uint32_t ackn;
	char *pld = NULL;
	char *strsgm = NULL;
	char *input = NULL;
	char *userbuff = NULL;
	char *strinbox = NULL;
	size_t rcvsize;

	printf("# Creating inbox with isn=%u wnds=%u #\n", (uint32_t) ISN, (uint32_t) WNDSIZE);

	inbox = createInbox(ISN, WNDSIZE);

	printf("# Inbox to string #\n");

	strinbox = inboxToString(inbox);

	printf("%s\n\n", strinbox);

	free(strinbox);

	while (1) {

		printf("# Submitting your segment to inbox (type 'quit' to exit test) #\n");

		input = getUserInput("[CTRL]>");

		if (strcmp(input, "quit") == 0) {		
			free(input);
			break;
		}

		ctrl = (uint8_t) atoi(input);

		free(input);

		input = getUserInput("[SEQN]>");

		seqn = (uint32_t) strtoul(input, NULL, 10) % RUDP_MAXSEQN;

		free(input);

		input = getUserInput("[ACKN]>");

		ackn = (uint32_t) strtoul(input, NULL, 10) % RUDP_MAXSEQN;

		free(input);

		input = getUserInput("[PLD]>");

		pld = stringDuplication(input);

		free(input);

		sgm = createSegment(ctrl, 0, 0, seqn, ackn, pld);

		free(pld);

		strsgm = segmentToString(sgm);

		printf("# Submitting segment to inbox: %s #\n", strsgm);

		free(strsgm);

		submitSegmentToInbox(inbox, sgm);

		printf("# Inbox to string #\n");

		strinbox = inboxToString(inbox);

		printf("%s\n\n", strinbox);

		free(strinbox);

		printf("# Reading user buffer from inbox (type 0 to read nothing) #\n");

		input = getUserInput("[RCVSIZE]>");

		rcvsize = (size_t) atoi(input);

		free(input);

		userbuff = readUserBuffer(inbox, rcvsize);

		printf("%s\n", userbuff);

		free(userbuff);

		printf("# Inbox to string #\n");

		strinbox = inboxToString(inbox);

		printf("%s\n\n", strinbox);

		free(strinbox);

		printf("\n");
	}			

	printf("# Freeing inbox #\n");

	freeInbox(inbox);	

	exit(EXIT_SUCCESS);
}
