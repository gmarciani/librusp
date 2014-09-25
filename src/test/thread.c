#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h> 
#include "../common/util.h"

#define MAX_INPIPE 2;

typedef struct mailbox_t {
	char *outbox;
	size_t outboxs;
	char *
	char *inbox;
	size_t inboxs;
} mailbox_t;

void send(mailbox_t *mail, const char *str);

static void *processOutbox(void *arg);

int main(void) {
	mailbox_t mail;

	send(&mail, "12345678910");
	free(input);
	
	return(EXIT_SUCCESS);	
}

void send(mailbox_t *mail, const char *str) {
	printf("Sending: %s\n", str);
	pthread_t snd;
	void *sndres;

	mail->outbox = stringDuplication(str);
	mail->outboxs = strlen(mail->outbox);

	printf("Oubox: %s\n", mail->outbox);

	if (pthread_create(&snd, NULL, processOutbox, mail) != 0) {
		fprintf(stderr, "Cannot create sender thread.\n");
		exit(EXIT_FAILURE);
	}

	if (pthread_join(snd, &sndres) != 0) {
		fprintf(stderr, "Cannot join sender thread.\n");
		exit(EXIT_FAILURE);
	}

	printf("%ld bytes correctly sent!\n", (long) sndres);	
	printf("Oubox: %s\n", mail->outbox);
}

static void *processOutbox(void *arg) {
	mailbox_t *mail = (mailbox_t *) arg;
	long int sent = 0;
	int i = 0

	while (mail->outboxs >= 0) {
		for (i = 0; i < mail->outboxs && i < ; i++) {
			mail->outbox[i] = '\0';
			mail->outboxs--;
			sent++;
		}
	}	

	return (void *) sent;
}
