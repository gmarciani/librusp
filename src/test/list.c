#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../util/list.h"

static List POOL = LIST_INITIALIZER;

static void *checkerFunc(void *arg);

static void *writerEvenFunc(void *arg);

static void *writerOddFunc(void *arg);

int main(void) {
	pthread_t writerEven, writerOdd, checker;

	pthread_create(&writerEven, NULL, writerEvenFunc, NULL);

	pthread_create(&writerOdd, NULL, writerEvenFunc, NULL);

	pthread_create(&checker, NULL, writerEvenFunc, NULL);

	pthread_join(writerOne, NULL);

	pthread_join(writerTwo, NULL);

	pthread_join(readerOne, NULL);

	pthread_join(readerTwo, NULL);
	
}

static void *checkerFunc(void *arg) {
	
	
}

static void *writerEvenFunc(void *arg) {
	int *element;

	element = malloc(sizeof(int));

	*element = 0;

	if (getListSize(&POOL) == 0)
		addElement(&POOL, (void *) element, sizeof(int));

	while (getListSize(&POOL) <= 100) {
		
	}
}

static void *writerOddFunc(void *arg) {

}
