#include <stdlib.h>
#include <stdio.h>
#include "../util/stringutil.h"

int main(void) {
	Buffer *buff = NULL;
	char *strbuff = NULL;
	char *input = NULL;
	char *output = NULL;
	size_t size;

	printf("# Creating buffer\n");

	buff = createBuffer();

	printf("# Buffer to string:\n");

	strbuff = bufferToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);

	while (1) {

		printf("# Writing user input to buffer (type 'quit' to exit test):\n");

		input = getUserInput("[DATA]>");

		if (strcmp(input, "quit") == 0) {

			free(input);

			break;
		}

		writeToBuffer(buff, input, strlen(input));

		free(input);

		printf("# Buffer to string: #\n");

		strbuff = bufferToString(buff);

		printf("%s\n", strbuff);

		free(strbuff);

		printf("# Reading from buffer:\n");

		input = getUserInput("[SIZE]>");

		size = (size_t) atoi(input);

		free(input);

		output = readFromBuffer(buff, size);

		printf("%s\n", output);

		free(output);

		printf("# Buffer to string:\n");

		strbuff = bufferToString(buff);

		printf("%s\n", strbuff);

		free(strbuff);

		printf("\n");
	}

	printf("# Freeing buffer\n");

	freeBuffer(buff);

	printf("Buffer freed\n");

	exit(EXIT_FAILURE);
}
