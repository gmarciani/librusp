#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../../util/macroutil.h"

static void voidfoo(void) {
	printf("VoidFoo\n");
}

static void argfoo(const int num, const char *string) {
	printf("argfoo: %d %s\n", num, string);
}

int main(void) {

	DBGPRINT(1, "debug message: %s", "true");

	DBGPRINT(0, "debug message: %s", "false");

	DBGFUNC(1, voidfoo());

	DBGFUNC(0, voidfoo());

	DBGFUNC(1, argfoo(1, "one"));

	ERREXIT("error");
}
