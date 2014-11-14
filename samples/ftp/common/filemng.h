#ifndef FILEMNG_H_
#define FILEMNG_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "util.h"

#define DIR_PERM 	(S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH)
#define FILE_PERM	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH)


// CONTENT EXPLORATION

int exploreDirectory(const char *dirpath, char ***list, int *numItems);


// FILES

int isFile(const char *filepath);

int mkFile(const char *filepath, const char *data);

int rmFile(const char *filepath);

int cpFile(const char *srcFilepath, const char *dstFilepath);

int mvFile(const char *srcFilepath, const char *dstFilepath);

int fileSerialization(const char *filepath, char **serializedFile);


// DIRECTORIES

int isDirectory(const char *dirpath);

int mkDirectory(const char *dirpath);

int rmDirectory(const char *dirpath);

int cpDirectory(const char *srcDirpath, const char *dstDirpath);

int mvDirectory(const char *srcDirpath, const char *dstDirpath);

int changeRoot(const char *rootpath);

char *getCwd(void);

int changeCwd(const char *newpath);


// UTILS

char *getFilename(const char *path);

char *concatPath(const char *path, const char *subpath);

#endif /* FILEMNG_H_ */
