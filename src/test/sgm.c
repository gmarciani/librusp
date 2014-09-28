#include "../core/rudpsegment.h"

int main(void) {
	Segment sgmOne, sgmTwo;
	char *strsgm = NULL;
	char *ssgm = NULL;
	int i;

	puts("# Creating SYNACK segments with urgp=5 wnds=324, seqn=51, ackn=22, pld=Hello World! #");

	sgmOne = createSegment(RUDP_SYN | RUDP_ACK, 5, 324, 51, 22, "Hello world!");

	strsgm = segmentToString(sgmOne);

	puts("# Segment to string #");

	printf("%s\n", strsgm);

	free(strsgm);

	puts("# Serialized segment #");	

	ssgm = serializeSegment(sgmOne);

	printf("%s\n", ssgm);

	puts("# Deserializing segment #");

	sgmTwo = deserializeSegment(ssgm);

	free(ssgm);

	if (sgmOne.hdr.vers == sgmTwo.hdr.vers &&
		sgmOne.hdr.ctrl == sgmTwo.hdr.ctrl &&
		sgmOne.hdr.urgp == sgmTwo.hdr.urgp &&
		sgmOne.hdr.plds == sgmTwo.hdr.plds &&
		sgmOne.hdr.wnds == sgmTwo.hdr.wnds &&
		sgmOne.hdr.seqn == sgmTwo.hdr.seqn &&
		sgmOne.hdr.ackn == sgmTwo.hdr.ackn) {
		puts("Header match SUCCESS!");
		for (i = 0; i < sgmOne.hdr.plds; i++) {
			if (sgmOne.pld[i] != sgmTwo.pld[i]) {
				puts("Payload match FAILURE!");
				exit(EXIT_FAILURE);
			}
		}
		puts("Payload match SUCCESS!");
	} else {
		puts("Header match FAILURE");
	}

	exit(EXIT_SUCCESS);
}
