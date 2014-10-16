#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>
#include "../../core/buffer/sgmbuff.h"
#include "../../util/threadutil.h"
#include "../../util/macroutil.h"

#define NUM_ELEMENTS 10
#define ISN (uint32_t) 0
#define PLD "Hello"

static SgmBuff *buff;

static void creation(void);

static void stringRepresentation(void);

static void insertion(void);

static void removal(void);

static void deallocation(void);

int main(void) {

	creation();

	stringRepresentation();

	insertion();	

	stringRepresentation();

	removal();

	stringRepresentation();

	deallocation();

	exit(EXIT_SUCCESS);
}

static void creation(void) {
	printf("# Creating segment buffer..");

	buff = createSgmBuff();

	printf("OK\n");
}

static void stringRepresentation(void) {
	char *strbuff = NULL;

	printf("# Segment buffer to string...\n");

	strbuff = sgmBuffToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);
}

static void insertion(void) {
	Segment sortsgm[NUM_ELEMENTS], sgm;
	SgmBuffElem *curr = NULL;
	uint32_t seqn = ISN;
	int i;

	printf("# Adding %d segments to segment buffer, with ISN %u and payload %s...", NUM_ELEMENTS, ISN, PLD);

	for (i = 0; i < NUM_ELEMENTS; i++) {
	
		sgm = createSegment(RUDP_ACK, 0, 0, seqn, i, PLD);

		sortsgm[i] = sgm;

		seqn = RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds);
	}

	for (i = NUM_ELEMENTS - 1; i >= 0; i--) {

		addSgmBuff(buff, sortsgm[i]);

		assert(isEqualSegment(sgm, findSgmBuffSeqn(buff, sgm.hdr.seqn)->segment) &&
				isEqualSegment(sgm, findSgmBuffAckn(buff, RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds))->segment));
	}

	curr = buff->head;

	while (curr) {
		if (curr->next)
			assert(curr->segment.hdr.seqn <= curr->next->segment.hdr.seqn);

		curr = curr->next;
	}

	printf("OK\n");
}

static void removal(void) {
	printf("# Removing segment with seqn 0 and segment acked by ackn 25...");

	removeSgmBuff(buff, findSgmBuffSeqn(buff, 0));

	removeSgmBuff(buff, findSgmBuffAckn(buff, 25));

	assert(!findSgmBuffSeqn(buff, 0) && !findSgmBuffAckn(buff, 25));

	printf("OK\n");
}

static void deallocation(void) {
	printf("# Freeing segment buffer...");

	freeSgmBuff(buff);

	printf("OK\n");
}
