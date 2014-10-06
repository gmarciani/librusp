#ifndef MATHUTIL_H_
#define MATHUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <openssl/md5.h>

#define MAX_UINT32 4294967295

uint32_t getRandom32(void);

uint8_t getRandomBit(const double onprob);

uint32_t getMD5(const char *input);

#endif /* MATHUTIL_H_ */
