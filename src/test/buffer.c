#include <stdlib.h>
#include <stdio.h>
#include "../util/stringutil.h"

int main(void) {
	Buffer *buff = NULL;
	char *strbuff = NULL;
	char *input = NULL;
	char *output = NULL;
	size_t size;

	printf("# Creating buffer #\n");

	buff = createBuffer();

	printf("# Getting buffer string representation #\n");

	strbuff = bufferToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);

	while (1) {

		printf("# Writing user input to buffer (type 'quit' to exit test) #\n");

		input = getUserInput("[DATA]>");

		if (strcmp(input, "quit") == 0) {

			free(input);

			break;
		}

		writeToBuffer(buff, input, strlen(input));

		free(input);

		printf("# Getting buffer string representation #\n");

		strbuff = bufferToString(buff);

		printf("%s\n", strbuff);

		free(strbuff);

		printf("# Reading from buffer #\n");

		input = getUserInput("[SIZE]>");

		size = (size_t) atoi(input);

		free(input);

		output = readFromBuffer(buff, size);

		printf("%s\n", output);

		free(output);

		printf("# Getting buffer string representation #\n");

		strbuff = bufferToString(buff);

		printf("%s\n", strbuff);

		free(strbuff);

		printf("\n");
	}

	printf("# Freeing buffer #\n");

	freeBuffer(buff);

	exit(EXIT_FAILURE);
}
