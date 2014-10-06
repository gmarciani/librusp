#include <stdlib.h>
#include <stdio.h>
#include "../core/rudpsegment.h"

int main(void) {
	Stream *stream = NULL;
	Segment sgmOne, sgmTwo;
	char *strsgm = NULL;
	char *ssgm = NULL;
	char *msg = "Hello World! I'm happy! Stay hungry, stay foolish, folks!";
	char *strstream = NULL;
	int i;

	printf("# Creating SYNACK segments (PLDS=%d) with urgp=5 wnds=324, seqn=51, ackn=22, pld=%s #\n", RUDP_PLDS, msg);

	sgmOne = createSegment(RUDP_SYN | RUDP_ACK, 5, 324, 51, 22, msg);

	strsgm = segmentToString(sgmOne);

	printf("# Segment to string:\n");

	printf("%s\n", strsgm);

	free(strsgm);

	printf("# Segment serialization\n");	

	ssgm = serializeSegment(sgmOne);

	printf("%s\n", ssgm);

	printf("# Segment deserialization:\n");

	sgmTwo = deserializeSegment(ssgm);

	free(ssgm);

	printf("# Segment matching\n");

	if (sgmOne.hdr.vers == sgmTwo.hdr.vers &&
		sgmOne.hdr.ctrl == sgmTwo.hdr.ctrl &&
		sgmOne.hdr.urgp == sgmTwo.hdr.urgp &&
		sgmOne.hdr.plds == sgmTwo.hdr.plds &&
		sgmOne.hdr.wnds == sgmTwo.hdr.wnds &&
		sgmOne.hdr.seqn == sgmTwo.hdr.seqn &&
		sgmOne.hdr.ackn == sgmTwo.hdr.ackn) {

		printf("Header match SUCCESS!\n");

		for (i = 0; i < sgmOne.hdr.plds; i++) {

			if (sgmOne.pld[i] != sgmTwo.pld[i]) {

				printf("Payload match FAILURE!\n");

				exit(EXIT_FAILURE);

			}
		}

		printf("Payload match SUCCESS!\n");

	} else {

		printf("Header match FAILURE\n");

	}

	printf("# Creating stream (PLDS=%d) with message=%s #\n", RUDP_PLDS, msg);

	stream = createStream(msg);

	printf("# Stream to string:\n");

	strstream = streamToString(stream);

	printf("%s\n", strstream);

	free(strstream);

	printf("# Cleaning stream\n");

	cleanStream(stream);

	printf("Stream cleaned\n");

	printf("# Stream to string #\n");

	strstream = streamToString(stream);

	printf("%s\n", strstream);

	free(strstream);

	printf("# Freeing stream\n");

	freeStream(stream);

	printf("Stream freed\n");

	exit(EXIT_SUCCESS);
}
