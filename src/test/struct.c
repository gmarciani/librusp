#include <stdlib.h>
#include <stdio.h>

typedef struct Struct {
	int number;
	char *string;
} Struct;

Struct getStruct();

int main(void) {
	Struct mystruct;

	mystruct = getStruct();

	printf("Mystruct: %d %s\n", mystruct.number, mystruct.string);

	exit(EXIT_SUCCESS);
}

Struct getStruct() {
	Struct mystruct;

	mystruct.number = 10;
	
	mystruct.string = malloc(sizeof(char) * 10);

	mystruct.string[0] = 'a';
	mystruct.string[1] = 'b';
	mystruct.string[2] = '\0';

	return mystruct;
}
