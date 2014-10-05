#include "stringutil.h"

/* BUFFER MANAGEMENT */

Buffer *createBuffer(void) {
	Buffer *buff = NULL;

	if (!(buff = malloc(sizeof(Buffer))) ||
		!(buff->content = malloc(sizeof(char))))
		ERREXIT("Cannot allocate memory for buffer.");

	buff->csize = 0;

	buff->bsize = 1;

	return buff;
}

void freeBuffer(Buffer *buff) {
	
	free(buff->content);

	buff->csize = 0;

	buff->bsize = 0;

	free(buff);
}

char *getBuffer(Buffer *buff, const size_t size) {
	char *str = NULL;
	size_t sizeToCopy, i;

	sizeToCopy = (size < buff->csize) ? size : buff->csize;

	if (!(str = malloc(sizeof(char) * (sizeToCopy + 1))))
		ERREXIT("Cannot allocate memory for buffer get.");

	for (i = 0; i < sizeToCopy; i++) 
		str[i] = buff->content[i];

	str[sizeToCopy] = '\0';

	return str;
}

char *readFromBuffer(Buffer *buff, const size_t size) {
	char *str = NULL;
	size_t sizeToCopy;

	sizeToCopy = (size < buff->csize) ? size : buff->csize;

	str = getBuffer(buff, sizeToCopy);

	if (sizeToCopy != 0) {

		memmove(buff->content, buff->content + sizeToCopy, sizeof(char) * (buff->csize - sizeToCopy));

		buff->csize -= sizeToCopy;

		if (buff->csize <= (buff->bsize / 4)) {

			buff->bsize /= 2;

			if (!(buff->content = realloc(buff->content, sizeof(char) * buff->bsize)))
				ERREXIT("Cannot reallocate memory for buffer.");
		}
	}

	return str;
}

void writeToBuffer(Buffer *buff, const char *str, const size_t size) {
	size_t i;

	if (size == 0)
		return;

	if ((buff->csize + size) >= (buff->bsize / 2)) {

		if ((buff->csize + size) * 2 <= BUFFER_DHBREAKPOINT) {
			
			buff->bsize = (buff->csize + size) * 2;

			if (!(buff->content = realloc(buff->content, sizeof(char) * buff->bsize)))
				ERREXIT("Cannot reallocate memory for buffer.");	
		} else {

			buff->bsize = (buff->csize + size < BUFFER_DHBREAKPOINT) ? BUFFER_DHBREAKPOINT : buff->csize + size;

			if (!(buff->content = realloc(buff->content, sizeof(char) * buff->bsize)))
				ERREXIT("Cannot reallocate memory for buffer.");
		}		
	}

	for (i = 0 ; i < size; i++) 
		buff->content[buff->csize + i] = str[i];

	buff->csize += size;
}

char *bufferToString(Buffer *buff) {
	char *strbuff = NULL;
	char *strcontent = NULL;

	strcontent = getBuffer(buff, buff->csize);

	if (!(strbuff = malloc(sizeof(char) * (43 + strlen(strcontent) + 1))))
		ERREXIT("Cannot allocate memory for buffer string representation.");

	sprintf(strbuff, "bsize:%zu csize:%zu content:%s", buff->bsize, buff->csize, strcontent);

	free(strcontent);

	return strbuff;
}

/* STRING MANAGEMENT */

char *stringDuplication(const char *src) {
	char *dest = NULL;
	size_t srcSize;

	srcSize = strlen(src);

	if (!(dest = malloc(sizeof(char) * (srcSize + 1))))
		ERREXIT("Cannot allocate memory for string duplication.");

	dest[0] = '\0';
	
	dest = strcat(dest, src);

	return dest;
}

char *stringNDuplication(const char *src, const size_t size) {
	char *dest = NULL;
	size_t srcSize;
	size_t sizeToCopy;	

	srcSize = strlen(src);

	sizeToCopy = (srcSize >= size) ? size : srcSize;

	if (!(dest = malloc(sizeof(char) * (sizeToCopy + 1))))
		ERREXIT("Cannot allocate memory for string n duplication.");

	dest[0] = '\0';
	
	dest = strncat(dest, src, sizeToCopy);

	return dest;
}

char *stringConcatenation(const char *srcOne, const char *srcTwo) {
	char *dest = NULL;
	size_t srcSize;

	srcSize = strlen(srcOne) + strlen(srcTwo);

	if (!(dest = malloc(sizeof(char) * (srcSize + 1))))
		ERREXIT("Cannot allocate memory for string concatenation.");

	dest[0] = '\0';

	dest = strcat(dest, srcOne);

	dest = strcat(dest, srcTwo);

	return dest;
}

/* STRING SPLITTING */

char **splitStringByDelimiter(const char *src, const char *delim, int *numSubstr) {
	char **substr = NULL;
	char *temp = NULL;
	char *token = NULL;

	temp = stringDuplication(src);

	if (!(substr = malloc(sizeof(char *))))
		ERREXIT("Cannot allocate memory for string split by delimiter.");

	for (token = strtok(temp, delim), *numSubstr = 0; token != NULL; token = strtok(NULL, delim)) {	
		*numSubstr += 1;
		if (!(substr = realloc(substr, sizeof(char *) * *numSubstr)))
			ERREXIT("Cannot reallocate memory for string split by delimiter.");

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

	if (!(substr = malloc(sizeof(char *) * numSubstr))) 
		ERREXIT("Cannot allocate memory for string split by n delimiter.");

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
	char **substr;
	size_t srcsize;
	int i;

	srcsize = strlen(src);

	*numSubstr = (srcsize % size == 0) ? (srcsize / size) : ((srcsize / size) + 1);

	if (!(substr = malloc(sizeof(char *) * *numSubstr)))
		ERREXIT("Cannot allocate memory for string split by size.");
	
	for (i = 0; i < *numSubstr; i++)
		substr[i] = stringNDuplication(src + (i * size), size);

	return substr;
}

char **splitStringBySection(const char *src, const size_t *ssize, const int numsubstr) {
	char **substr;
	int processed, i;

	if (!(substr = malloc(sizeof(char *) * numsubstr)))
		ERREXIT("Cannot allocate memory for string split by section.");

	for (i = 0, processed = 0; i < numsubstr; i++, processed += ssize[i-1])
		substr[i] = stringNDuplication(src + processed, ssize[i]);

	return substr;
}

/* ARRAY (DE)SERIALIZATION */

char *arraySerialization(char **array, const int numItems, const char *delim) {
	char *sarray;
	size_t sarraysize = 0;
	int i;

	for (i = 0; i < numItems; i++)
		sarraysize += (strlen(array[i]) + 1);

	if (!(sarray = malloc(sizeof(char) * (sarraysize + 1))))
		ERREXIT("Cannot allocate memory for array serialization.");

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

/* VARIOUS */

char *getTime(void) {
	char *str = NULL;
	char localTime[10];
	size_t size = 40;
	
	struct timeval time;

	if (!(str = malloc(sizeof(char) * size)))
		ERREXIT("Cannot allocate memory for time string representation.");
	
	gettimeofday(&time, NULL);

	struct tm *local = localtime(&time.tv_sec);

	strftime(localTime, 10, "%H:%M:%S", local);

	snprintf(str, size, "%s:%ld", localTime, time.tv_usec);
	
	return str;
}

char *getUserInput(const char *descr) {
	char *input;
	size_t size = 2048;	

	if (!(input = malloc(sizeof(char) * size)))
		ERREXIT("Cannot allocate memory for user input.");

	input[0] = '\0';

	printf("\n%s", descr);

	fflush(stdout);

  	if (getline(&input, &size, stdin) == -1)
		ERREXIT("Cannot get line for user input.");

	input[strlen(input) - 1] = '\0';

	return input;
}
