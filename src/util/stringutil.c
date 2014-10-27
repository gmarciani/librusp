#include "stringutil.h"

/* STRING MANAGEMENT */

char *stringDuplication(const char *src) {
	char *dest = NULL;

	if (!(dest = strdup(src)))
		ERREXIT("Cannot duplicate string");

	return dest;
}

char *stringNDuplication(const char *src, const size_t size) {
	char *dest = NULL;

	if (!(dest = strndup(src, size)))
		ERREXIT("Cannot n-duplicate string");

	return dest;
}

char *stringConcatenation(const char *srcone, const char *srctwo) {
	char *dest = NULL;

	if (!(dest = malloc(sizeof(char) * (strlen(srcone) + strlen(srctwo) + 1))))
		ERREXIT("Cannot allocate memory for string concatenation");

	dest[0] = '\0';

	strcat(dest, srcone);

	strcat(dest, srctwo);

	return dest;
}

/* STRING SPLITTING */

char **splitStringByDelimiter(const char *src, const char *delim, int *substrs) {
	char **substr = NULL;
	char *temp = NULL;
	char *token = NULL;

	temp = stringDuplication(src);

	if (!(substr = malloc(sizeof(char *))))
		ERREXIT("Cannot allocate memory for string split by delimiter.");

	for (token = strtok(temp, delim), *substrs = 0; token != NULL; token = strtok(NULL, delim)) {	
		*substrs += 1;
		if (!(substr = realloc(substr, sizeof(char *) * *substrs)))
			ERREXIT("Cannot reallocate memory for string split by delimiter.");

		substr[*substrs - 1] = stringDuplication(token);
	}

	free(temp);

	return substr;
}

char **splitStringNByDelimiter(const char *src, const char *delim, const int substrs) {
	char **substr = NULL;
	char *temp, *tempp = NULL;
	char *delimMatch = NULL;
	size_t delimSize = strlen(delim);
	size_t tokenSize;
	int effNumSubstr = 0;
	int i;

	temp = stringDuplication(src);

	tempp = temp;

	if (!(substr = malloc(sizeof(char *) * substrs))) 
		ERREXIT("Cannot allocate memory for string split by n delimiter.");

	for (delimMatch = strstr(temp, delim); delimMatch != NULL; delimMatch = strstr(temp, delim)) {	

		effNumSubstr ++;

		tokenSize = delimMatch - temp;

		substr[effNumSubstr - 1] = stringNDuplication(temp, tokenSize);

		temp = delimMatch + delimSize;

		if (effNumSubstr + 1 == substrs)
			break;
	}
	
	effNumSubstr ++;

	substr[effNumSubstr - 1] = stringDuplication(temp);

	for (i = effNumSubstr; i < substrs; i++)
		substr[i] = stringDuplication("");

	free(tempp);

	return substr;
}

char **splitStringBySize(const char *src, const size_t size, int *substrs) {
	char **substr;
	size_t srcsize;
	int i;

	srcsize = strlen(src);

	*substrs = (srcsize % size == 0) ? (srcsize / size) : ((srcsize / size) + 1);

	if (!(substr = malloc(sizeof(char *) * *substrs)))
		ERREXIT("Cannot allocate memory for string split by size.");
	
	for (i = 0; i < *substrs; i++)
		substr[i] = stringNDuplication(src + (i * size), size);

	return substr;
}

char **splitStringBySection(const char *src, const size_t *ssize, const int substrs) {
	char **substr;
	int processed, i;

	if (!(substr = malloc(sizeof(char *) * substrs)))
		ERREXIT("Cannot allocate memory for string split by section.");

	for (i = 0, processed = 0; i < substrs; i++, processed += ssize[i-1])
		substr[i] = stringNDuplication(src + processed, ssize[i]);

	return substr;
}

/* ARRAY (DE)SERIALIZATION */

char *arraySerialization(char **array, const int items, const char *delim) {
	char *sarray;
	size_t sarrays, elems, delims;
	int i, j;

	delims = strlen(delim);

	sarrays = 0;

	for (i = 0; i < items; i++)
		sarrays += (strlen(array[i]) + delims);

	if (!(sarray = malloc(sizeof(char) * (sarrays + 1))))
		ERREXIT("Cannot allocate memory for array serialization.");

	j = 0;

	for (i = 0; i < items; i++) {

		elems = strlen(array[i]);

		memcpy(sarray + j, array[i], sizeof(char) * elems);

		j += elems;

		memcpy(sarray + j, delim, sizeof(char) * delims);

		j += delims;
	}

	sarray[j] = '\0';

	return sarray;
}

char **arrayDeserialization(const char *sarray, const char *delim, int *items) {
	return splitStringByDelimiter(sarray, delim, items);
}

/* VARIOUS */

char *getUserInput(const char *descr) {
	char *input;
	size_t size = 2048;	

	if (!(input = malloc(sizeof(char) * size)))
		ERREXIT("Cannot allocate memory for user input.");

	input[0] = '\0';

	fflush(stdout);

	printf("\n%s", descr);

  	if (getline(&input, &size, stdin) == -1)
		ERREXIT("Cannot get line for user input.");

	input[strlen(input) - 1] = '\0';

	return input;
}
