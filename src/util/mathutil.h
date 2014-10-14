#ifndef MATHUTIL_H_
#define MATHUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <openssl/md5.h>
#include "macroutil.h"

unsigned long getRandomUL(void);

unsigned short getRandomBit(const double onprob);

unsigned long getMD5(const char *input);

#endif /* MATHUTIL_H_ */
