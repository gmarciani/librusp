#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h> 

static void *threadFunc(void *arg);

int main(void) {
	pthread_t tid;

	printf("Process tid: %lu\n", (unsigned long) pthread_self());

	pthread_create(&tid, NULL, threadFunc, NULL);
	
	pthread_join(tid, NULL);

	printf("Process tid: %lu\n", (unsigned long) pthread_self());
		
	return(EXIT_SUCCESS);	
}

static void *threadFunc(void *arg) {
	printf("Inside thread function\n");
	printf("Thread tid: %lu\n", (unsigned long) pthread_self());
	return (void *) NULL;
}
