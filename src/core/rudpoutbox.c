#include "rudpoutbox.h"

static void slideOutboxWindow(Outbox *outbox);

static void removeOutboxElement(Outbox *outbox, OutboxElement *elem);

Outbox *createOutbox(const uint32_t isn, const uint32_t wnds) {
	Outbox *outbox;

	if (wnds == 0)
		ERREXIT("Cannot create outbox with window size set to zero.");

	if (!(outbox = malloc(sizeof(Outbox))))
		ERREXIT("Cannot allocate memory for outbox.");

	outbox->head = NULL;

	outbox->tail = NULL;

	outbox->size = 0;

	outbox->wndb = NULL;

	outbox->wnde = NULL;

	outbox->wnds = wnds;
	
	outbox->awnds = 0;	

	outbox->nextseqn = isn;	

	return outbox;
}

void freeOutbox(Outbox *outbox) {
	OutboxElement *curr = NULL;

	curr = outbox->head;

	while (curr) {

		if (curr->prev) {

			free(curr->prev->segment);

			free(curr->prev);
		}

		if (curr == outbox->tail) {

			free(curr->segment);

			free(curr);

			break;
		}

		curr = curr->next;
	}

	free(outbox);
}

void submitSegmentToOutbox(Outbox *outbox, const Segment sgm) {
	OutboxElement *new = NULL;

	if (!(new = malloc(sizeof(OutboxElement))))
		ERREXIT("Cannot allocate memory for outbox element insertion.");

	if (!(new->segment = malloc(sizeof(Segment))))
		ERREXIT("Cannot allocate memory for outbox segment insertion.");

	new->status = RUDP_UNACKED;	

	new->segment = memcpy(new->segment, &sgm, sizeof(Segment));

	new->segment->hdr.seqn = outbox->nextseqn;

	outbox->nextseqn = RUDP_NXTSEQN(outbox->nextseqn, new->segment->hdr.plds);

	if (outbox->size == 0) {

		new->prev = NULL;

		new->next = NULL;

		outbox->head = new;

		outbox->tail = new;

		outbox->wndb = new;

		outbox->wnde = new;

		outbox->awnds++;

	} else {

		new->prev = outbox->tail;

		new->next = NULL;

		outbox->tail->next = new;

		outbox->tail = new;

		if (outbox->awnds < outbox->wnds) {

			outbox->wnde = new;

			outbox->awnds++;
		}			
	}	
	
	outbox->size++;	
}

void submitAckToOutbox(Outbox *outbox, const uint32_t ackn) {
	OutboxElement *curr;

	curr = outbox->wndb;

	while (curr) {

		if (outbox->wnde)
			if (curr == outbox->wnde->next)
				break;	
	
		if (ackn == RUDP_NXTSEQN(curr->segment->hdr.seqn, curr->segment->hdr.plds)) {

			curr->status = RUDP_ACKED;

			slideOutboxWindow(outbox);

			break;
		}

		curr = curr->next;
	}
}

Segment *getRetransmittableSegments(Outbox *outbox, uint32_t *retransno) {
	Segment *retrans = NULL;
	OutboxElement *curr = NULL;

	if (!(retrans = malloc(sizeof(Segment) * outbox->awnds)))
		ERREXIT("Cannot allocate memory for array of retransmittable segments from outbox.");
	
	curr = outbox->wndb;

	*retransno = 0;

	while (curr) {

		if (outbox->wnde)
			if (curr == outbox->wnde->next)
				break;

		if (curr->status == RUDP_UNACKED) {

			if (!memcpy(retrans + *retransno, curr->segment, sizeof(Segment))) 
				ERREXIT("Cannot copy segment from outbox to retransmittable segments.");

			*retransno += 1;
		}

		curr = curr->next;
	}

	return retrans;
}

char *outboxToString(Outbox *outbox) {
	OutboxElement *curr = NULL;
	char *stroutbox = NULL;
	char *strsgm = NULL;

	if (!(stroutbox = malloc(sizeof(char) * (77 + outbox->size * (RUDP_SGMSO + 5 + 1)))))
		ERREXIT("Cannot allocate string for outbox to string.");

	sprintf(stroutbox, "Outbox size:%u wnds:%u awnds:%u nextseqn:%u\n", outbox->size, outbox->wnds, outbox->awnds, outbox->nextseqn);	

	curr = outbox->head;

	while (curr) {

		if (curr == outbox->wndb)
			strcat(stroutbox, "<\t");
		else if (curr == outbox->wnde)
			strcat(stroutbox, ">\t");
		else
			strcat(stroutbox, " \t");

		if (curr->status == RUDP_ACKED)
			strcat(stroutbox, "A ");
		else
			strcat(stroutbox, "U ");

		strsgm = segmentToString(*(curr->segment));

		strcat(stroutbox, strsgm);

		strcat(stroutbox, "\n");

		curr = curr->next;

		free(strsgm);
	}

	return stroutbox;
}

static void slideOutboxWindow(Outbox *outbox) {
	while (outbox->wndb) {

		if (outbox->wndb->status != RUDP_ACKED)
			break;

		outbox->wndb = outbox->wndb->next;

		if (outbox->wnde == outbox->tail) {

			outbox->wnde = NULL;

			outbox->awnds--;

		} else if (outbox->wnde == NULL) {

			outbox->awnds--;

		} else {

			outbox->wnde = outbox->wnde->next;

		}

		if (outbox->wndb != NULL)
			removeOutboxElement(outbox, outbox->wndb->prev);
		else
			removeOutboxElement(outbox, outbox->head);	
	}
}

static void removeOutboxElement(Outbox *outbox, OutboxElement *elem) {
	if (!elem)
		return;

	if ((outbox->head == elem) && (outbox->tail == elem)) {

		outbox->head = NULL;

		outbox->tail = NULL;
				
	} else if (outbox->head == elem) {

		outbox->head = elem->next;

		outbox->head->prev = NULL;

	} else if (outbox->tail == elem) {

		outbox->tail = elem->prev;

		outbox->tail->next = NULL;

	} else {

		elem->prev->next = elem->next;

		elem->next->prev = elem->prev;		
	}

	free(elem->segment);

	free(elem);

	outbox->size--;
}
