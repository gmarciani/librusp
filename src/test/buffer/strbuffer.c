#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../../core/buffer/strbuff.h"
#include "../../util/macroutil.h"

#define ITERATIONS 10000000

#define STRING "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur? At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident, similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi optio cumque nihil impedit quo minus id quod maxime placeat facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus asperiores repellat."

#define STRINGSIZE strlen(STRING)

static Buffer *buff;

static void creation(void);

static void writeTo(void);

static void lookFrom(void);

static void readFrom(void);

static void rwProfile(void);

static void deallocation(void);

int main(void) {

	creation();

	writeTo();

	lookFrom();

	readFrom();

	rwProfile();	

	deallocation();

	exit(EXIT_FAILURE);
}

static void creation(void) {
	printf("# Creating buffer\n");

	buff = createBuffer();

	printf("SUCCESS\n");
}

static void writeTo(void) {
	printf("# Writing buffer\n");

	writeBuffer(buff, STRING, STRINGSIZE);

	if (strncmp(buff->content, STRING, buff->size) != 0)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");
}

static void lookFrom(void) {
	char *str = NULL;

	printf("# Looking buffer\n");

	str = lookBuffer(buff, 3);

	if (strcmp(str, "Lor") != 0 || buff->size != STRINGSIZE)
		ERREXIT("FAILURE");

	free(str);

	printf("SUCCESS\n");
}

static void readFrom(void) {
	char *str = NULL;

	printf("# Reading buffer\n");

	str = readBuffer(buff, 3);

	if (strcmp(str, "Lor") != 0 || buff->size != STRINGSIZE - 3)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	free(str);
}

static void rwProfile(void) {
	char *str = NULL;
	struct timespec start, end;
	long double elaps;	
	unsigned long iteration;

	printf("# Profiling writing/reading buffer (%d iteration on %zu bytes)\n", ITERATIONS, STRINGSIZE);

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 0; iteration < ITERATIONS; iteration++) {
		
		writeBuffer(buff, STRING, STRINGSIZE);

		str = readBuffer(buff, STRINGSIZE);

		free(str);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	elaps = ((long double)(end.tv_sec - start.tv_sec) + (long double)(end.tv_nsec - start.tv_nsec) / 1000000000.0);

	printf("elapsed: %LF seconds.\n", elaps);
}

static void deallocation(void) {
	printf("# Freeing buffer\n");

	freeBuffer(buff);

	printf("SUCCESS\n");
}