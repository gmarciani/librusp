#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../util/mathutil.h"

#define STRING "Hy folks! I'm Jack. Let's us enjoy MD5!"
 
int main(void) {
	uint32_t hash;

	printf("# Generating MD5 hashing for: %s\n", STRING);

	hash = getMD5(STRING);

	printf("%u\n", hash);
 
	exit(EXIT_SUCCESS);
}
