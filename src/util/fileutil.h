#ifndef FILEUTIL_H_
#define FILEUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "macroutil.h"

int openFile(const char *path, int flags);

void closeFile(const int fd);

void generateSampleFile(const int fd, const long size);

long long getFileSize(const int fd);

int isEqualFile(const int fdone, const int fdtwo);

#endif /* FILEUTIL_H_ */
