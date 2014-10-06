#ifndef MATHUTIL_H_
#define MATHUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#define MAX_UINT32 4294967295

uint32_t getRandom32(void);

uint8_t getRandomBit(const double onprob);

#endif /* MATHUTIL_H_ */
