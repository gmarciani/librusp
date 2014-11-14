#include "util.h"

char *getTime(void) {
	char *str = NULL;
	char localTime[10];
	size_t size = 40;
	
	struct timeval time;

	if (!(str = malloc(sizeof(char) * size))) {
		fprintf(stderr, "Error in string allocation for time.\n");
		exit(EXIT_FAILURE);
	}
	
	gettimeofday(&time, NULL);
	struct tm *local = localtime(&time.tv_sec);
	strftime(localTime, 10, "%H:%M:%S", local);
	snprintf(str, size, "%s:%ld", localTime, time.tv_usec);
	
	return str;
}

char *getUserInput(const char *descr) {
	char *input;
	size_t size = 2048;	

	if (!(input = malloc(sizeof(char) * size))) {
		fprintf(stderr, "Error in string allocation for user input.\n");
		exit(EXIT_FAILURE);
	}

	input[0] = '\0';

	printf("\n%s", descr);
	fflush(stdout);

  	if (getline(&input, &size, stdin) == -1) {
		fprintf(stderr, "Error in getline for user input.\n");
		exit(EXIT_FAILURE);
	}

	input[strlen(input) - 1] = '\0';

	return input;
}

char *stringDuplication(const char *src) {
	char *dest = NULL;
	size_t srcSize;

	srcSize = strlen(src);

	if (!(dest = malloc(sizeof(char) * (srcSize + 1)))) {
		fprintf(stderr, "Error in string allocation for duplication: src=%s.\n", src);
		exit(EXIT_FAILURE);
	}

	dest[0] = '\0';
	
	dest = strcat(dest, src);

	return dest;
}

char *stringNDuplication(const char *src, size_t size) {
	char *dest = NULL;
	size_t srcSize;
	size_t sizeToCopy;	

	srcSize = strlen(src);
	sizeToCopy = (srcSize >= size) ? size : srcSize;

	if (!(dest = malloc(sizeof(char) * (sizeToCopy + 1)))) {
		fprintf(stderr, "Error in string allocation for duplication: src=%s size=%zu.\n", src, size);
		exit(EXIT_FAILURE);
	}

	dest[0] = '\0';
	
	dest = strncat(dest, src, sizeToCopy);

	return dest;
}

char *stringConcatenation(const char *srcOne, const char *srcTwo) {
	char *dest = NULL;
	size_t srcSize;

	srcSize = strlen(srcOne) + strlen(srcTwo);

	if (!(dest = malloc(sizeof(char) * (srcSize + 1)))) {
		fprintf(stderr, "Error in string allocation for concatenation: srcOne=%s srcTwo=%s.\n", srcOne, srcTwo);
		exit(EXIT_FAILURE);
	}

	dest[0] = '\0';

	dest = strcat(dest, srcOne);
	dest = strcat(dest, srcTwo);

	return dest;
}

char **splitStringByDelimiter(const char *src, const char *delim, int *numSubstr) {
	char **substr = NULL;
	char *temp = NULL;
	char *token = NULL;

	temp = stringDuplication(src);

	if (!(substr = malloc(sizeof(char *)))) {
		fprintf(stderr, "Error in substrings allocation for split: src=%s delim=%s.\n", src, delim);
		exit(EXIT_FAILURE);
	}

	for (token = strtok(temp, delim), *numSubstr = 0; token != NULL; token = strtok(NULL, delim)) {	
		*numSubstr += 1;
		if (!(substr = realloc(substr, sizeof(char *) * *numSubstr))) {
			fprintf(stderr, "Error in substring %d reallocation for split: src=%s delim=%s.\n", *numSubstr, src, delim);
			exit(EXIT_FAILURE);
		}

		substr[*numSubstr - 1] = stringDuplication(token);
	}

	free(temp);

	return substr;
}

char **splitStringNByDelimiter(const char *src, const char *delim, const int numSubstr) {
	char **substr = NULL;
	char *temp, *tempp = NULL;
	char *delimMatch = NULL;
	size_t delimSize = strlen(delim);
	size_t tokenSize;
	int effNumSubstr = 0;
	int i;

	temp = stringDuplication(src);
	tempp = temp;

	if (!(substr = malloc(sizeof(char *) * numSubstr))) {
		fprintf(stderr, "Error in substrings allocation for split: src=%s delim=%s numSubstr=%d.\n", src, delim, numSubstr);
		exit(EXIT_FAILURE);
	}

	for (delimMatch = strstr(temp, delim); delimMatch != NULL; delimMatch = strstr(temp, delim)) {	
		effNumSubstr ++;
		tokenSize = delimMatch - temp;
		substr[effNumSubstr - 1] = stringNDuplication(temp, tokenSize);
		temp = delimMatch + delimSize;
		if (effNumSubstr + 1 == numSubstr)
			break;
	}
	
	effNumSubstr ++;
	substr[effNumSubstr - 1] = stringDuplication(temp);

	for (i = effNumSubstr; i < numSubstr; i++)
		substr[i] = stringDuplication("");

	free(tempp);

	return substr;
}

char **splitStringBySize(const char *src, const size_t size, int *numSubstr) {
	char **substr = NULL;
	size_t srcSize;

	srcSize = strlen(src);
	*numSubstr = (srcSize % size == 0) ? (srcSize / size) : ((srcSize / size) + 1);

	if (!(substr = malloc(sizeof(char *) * *numSubstr))) {
		fprintf(stderr, "Error in substrings allocation for split: src=%s size=%zu.\n", src, size);
		exit(EXIT_FAILURE);
	}

	int i;
	for (i = 0; i < *numSubstr; i++)
		substr[i] = stringNDuplication(src + (i * size), size);

	return substr;
}

char *arraySerialization(char **array, const int numItems, const char *delim) {
	char *sarray;
	size_t sarraysize = 0;
	int i;

	for (i = 0; i < numItems; i++)
		sarraysize += (strlen(array[i]) + 1);

	if (!(sarray = malloc(sizeof(char) * (sarraysize + 1)))) {
		fprintf(stderr, "Error in string allocation for array serialization.\n");
		exit(EXIT_FAILURE);
	}

	sarray[0] = '\0';

	for (i = 0; i < numItems; i++) {
		sarray = strcat(sarray, array[i]);
		if (i == (numItems -1))
			break;
		sarray = strcat(sarray, delim);
	}

	return sarray;
}

char **arrayDeserialization(const char *sarray, const char *delim, int *numItems) {
	return splitStringByDelimiter(sarray, delim, numItems);
}
