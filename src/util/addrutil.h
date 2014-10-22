#ifndef ADDRUTIL_H_
#define ADDRUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "macroutil.h"

#define ADDRIPV4_STR 22

struct sockaddr_in createAddress(const char *ip, const int port);

int isEqualAddress(const struct sockaddr_in addrOne, const struct sockaddr_in addrTwo);

void addressToString(const struct sockaddr_in addr, char *buff);

#endif /* ADDRUTIL_H_ */
