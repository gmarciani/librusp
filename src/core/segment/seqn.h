#ifndef SEQN_H_
#define SEQN_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/times.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../../util/addrutil.h"
#include "../../util/mathutil.h"
#include "../../util/macroutil.h"

#define RUDP_MAXSEQN (uint32_t) 4294967295

#define RUDP_NXTSEQN(seqn, plds) (((seqn) + (plds)) % RUDP_MAXSEQN)

#define RUDP_ISACKED(seqn, plds, ackn) ((ackn) == RUDP_NXTSEQN((seqn), (plds)))

int matchSequenceAgainstWindow(const uint32_t wndb, const uint32_t wnde, const uint32_t seqn);

uint32_t getRandomSequence(const struct sockaddr_in laddr, const struct sockaddr_in paddr);

#endif /* SEQN_H_ */
