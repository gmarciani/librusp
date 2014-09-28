#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "../util/stringutil.h"

#define RUDP_SYN 0b00000001
#define	RUDP_FIN 0b00000010
#define RUDP_RST 0b00000100
#define RUDP_ACK 0b00001000
#define RUDP_PSH 0b00010000	

void checkBitmask(const uint8_t bits);

void checkNotExclusiveBitmask(const uint8_t bits, const uint8_t mask);

void checkExclusiveBitmask(const uint8_t bits, const uint8_t mask);

int main(void) {
	char *input = NULL;
	uint8_t ctrlOne = 0b00001001;
	uint8_t ctrlTwo = RUDP_SYN | RUDP_ACK;
	uint8_t ctrlThree;

	puts("First ctrl declared binary (SYN, ACK)");

	printf("%03u\n", ctrlOne);

	checkBitmask(ctrlOne);

	checkExclusiveBitmask(ctrlOne, RUDP_SYN);

	checkExclusiveBitmask(ctrlOne, RUDP_SYN | RUDP_ACK);

	checkExclusiveBitmask(ctrlOne, RUDP_SYN | RUDP_ACK | RUDP_PSH);

	checkNotExclusiveBitmask(ctrlOne, RUDP_SYN);

	checkNotExclusiveBitmask(ctrlOne, RUDP_SYN | RUDP_ACK);

	checkNotExclusiveBitmask(ctrlOne, RUDP_SYN | RUDP_ACK | RUDP_PSH);

	puts("Second ctrl declared ORed (SYN, ACK)");

	printf("%03u\n", ctrlTwo);

	checkBitmask(ctrlTwo);

	checkExclusiveBitmask(ctrlTwo, RUDP_SYN);

	checkExclusiveBitmask(ctrlTwo, RUDP_SYN | RUDP_ACK);

	checkExclusiveBitmask(ctrlTwo, RUDP_SYN | RUDP_ACK | RUDP_PSH);

	checkNotExclusiveBitmask(ctrlTwo, RUDP_SYN);

	checkNotExclusiveBitmask(ctrlTwo, RUDP_SYN | RUDP_ACK);

	checkNotExclusiveBitmask(ctrlTwo, RUDP_SYN | RUDP_ACK | RUDP_PSH);

	puts("Third ctrl parsing your input");

	while (1) {
		input = getUserInput("[CTRL]>");

		if (strcmp(input, "quit") == 0)
			break;

		ctrlThree = (uint8_t) atoi(input);

		printf("%03u\n", ctrlThree);

		checkBitmask(ctrlThree);

		checkExclusiveBitmask(ctrlThree, RUDP_SYN);

		checkExclusiveBitmask(ctrlThree, RUDP_SYN | RUDP_ACK);

		checkExclusiveBitmask(ctrlThree, RUDP_SYN | RUDP_ACK | RUDP_PSH);

		checkNotExclusiveBitmask(ctrlThree, RUDP_SYN);

		checkNotExclusiveBitmask(ctrlThree, RUDP_SYN | RUDP_ACK);

		checkNotExclusiveBitmask(ctrlThree, RUDP_SYN | RUDP_ACK | RUDP_PSH);

		free(input);
	}		

	free(input);

	exit(EXIT_SUCCESS);
}

void checkBitmask(const uint8_t bits) {
	if (bits & RUDP_SYN)
		puts("SYN bit set!\n");
	else
		puts("SYN bit not set!\n");

	if (bits & RUDP_FIN)
		puts("FIN bit set!\n");
	else
		puts("FIN bit not set!\n");

	if (bits & RUDP_RST)
		puts("RST bit set!\n");
	else
		puts("RST bit not set!\n");

	if (bits & RUDP_ACK)
		puts("ACK bit set!\n");
	else
		puts("ACK bit not set!\n");

	if (bits & RUDP_PSH)
		puts("PSH bit set!\n");
	else
		puts("PSH bit not set!\n");
}

void checkNotExclusiveBitmask(const uint8_t bits, const uint8_t mask) {
	if (bits & mask) 
		puts("Not Exclusive Mask accepted!");
	else
		puts("Not Exclusive Mask NOT accepted!");
}

void checkExclusiveBitmask(const uint8_t bits, const uint8_t mask) {
	if (bits == mask) 
		puts("Exclusive Mask accepted!");
	else
		puts("Exclusive Mask NOT accepted!");
}
