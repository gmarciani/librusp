#ifndef STRINGUTIL_H_
#define STRINGUTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macroutil.h"

/* STRING MANAGEMENT */

char *stringDuplication(const char *src);

char *stringNDuplication(const char *src, const size_t size);

char *stringConcatenation(const char *srcone, const char *srctwo);

/* STRING SPLITTING */

char **splitStringByDelimiter(const char *src, const char *delim, int *substrs);

char **splitStringNByDelimiter(const char *src, const char *delim, const int substrs);

char **splitStringBySize(const char *src, const size_t size, int *substrs);

char **splitStringBySection(const char *src, const size_t *ssize, const int substrs);

/* ARRAY (DE)SERIALIZATION */

char *arraySerialization(char **array, const int items, const char *delim);

char **arrayDeserialization(const char *sarray, const char *delim, int *items);

/* VARIOUS */

char *getUserInput(const char *descr);

#endif /* STRINGUTIL_H_ */
