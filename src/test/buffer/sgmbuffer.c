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

static SgmBuff buff;

static void creation(void);

static void insertion(void);

static void removal(void);

static void deallocation(void);

int main(void) {

	creation();

	insertion();	

	removal();

	deallocation();

	exit(EXIT_SUCCESS);
}

static void creation(void) {
	printf("# Creating segment buffer..");

	initializeSgmBuff(&buff);

	printf("OK\n");
}

static void insertion(void) {
	Segment sgm;
	uint32_t seqn = ISN;
	int i;

	printf("# Adding %d segments to segment buffer, with ISN %u and payload %s...", NUM_ELEMENTS, ISN, PLD);

	for (i = 0; i < NUM_ELEMENTS; i++) {
	
		sgm = createSegment(RUDP_ACK, 0, strlen(PLD), 0, seqn, i, PLD);

		addSgmBuff(&buff, sgm, 0);

		assert(isEqualSegment(sgm, findSgmBuffSeqn(&buff, sgm.hdr.seqn)->segment) &&
				isEqualSegment(sgm, findSgmBuffAckn(&buff, RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds))->segment));

		seqn = RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds);
	}

	printf("OK\n");
}

static void removal(void) {
	printf("# Removing segment with seqn 0 and segment acked by ackn 25...");

	removeSgmBuff(&buff, findSgmBuffSeqn(&buff, 0));

	removeSgmBuff(&buff, findSgmBuffAckn(&buff, 25));

	assert(!findSgmBuffSeqn(&buff, 0) && !findSgmBuffAckn(&buff, 25));

	printf("OK\n");
}

static void deallocation(void) {
	printf("# Freeing segment buffer...");

	destroySgmBuff(&buff);

	printf("OK\n");
}
