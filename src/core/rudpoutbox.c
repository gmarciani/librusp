#include "rudpoutbox.h"

SegmentOutbox createOutbox(const unsigned long isn, const unsigned long wndsize) {
	SegmentOutbox outbox;

	if (wndsize == 0) {
		fprintf(stderr, "Cannot create outbox with window size to zero.\n");
		exit(EXIT_FAILURE);
	}

	outbox.head = NULL;
	outbox.tail = NULL;
	outbox.size = 0;
	outbox.wndbase = NULL;
	outbox.wndend = NULL;
	outbox.wndsize = wndsize;	
	outbox.awndsize = 0;	
	outbox.nextseqno = isn;	

	return outbox;
}

void freeOutbox(SegmentOutbox *outbox) {
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

	outbox->head = NULL;
	outbox->tail = NULL;	
	outbox->size = 0;
	outbox->wndbase = NULL;
	outbox->wndend = NULL;	
	outbox->wndsize = 0;
	outbox->awndsize = 0;	
	outbox->nextseqno = 0;
}

void submitSegmentToOutbox(SegmentOutbox *outbox, const Segment sgm) {
	OutboxElement *new = NULL;

	if (!(new = malloc(sizeof(OutboxElement)))) {
		fprintf(stderr, "Cannot allocate memory for outbox element insertion.\n");
		exit(EXIT_FAILURE);
	}	

	if (!(new->segment = malloc(sizeof(Segment)))) {
		fprintf(stderr, "Cannot allocate memory for outbox segment insertion.\n");
		exit(EXIT_FAILURE);
	}

	new->status = _RUDP_UNACKED;	

	new->segment = memcpy(new->segment, &sgm, sizeof(Segment));
	new->segment->hdr.seqno = outbox->nextseqno;
	outbox->nextseqno += (new->segment->hdr.plds == 0) ? 1 : new->segment->hdr.plds;

	if (outbox->size == 0) {
		new->prev = NULL;
		new->next = NULL;
		outbox->head = new;
		outbox->tail = new;
		outbox->wndbase = new;
		outbox->wndend = new;
		outbox->awndsize++;
	} else {
		new->prev = outbox->tail;
		new->next = NULL;
		outbox->tail->next = new;
		outbox->tail = new;
		if (outbox->awndsize < outbox->wndsize) {
			outbox->wndend = new;
			outbox->awndsize++;
		}			
	}	
	
	outbox->size++;	
}

void submitAckToOutbox(SegmentOutbox *outbox, const unsigned long ackno) {
	OutboxElement *curr;

	curr = outbox->wndbase;
	while (curr) {
		if (outbox->wndend)
			if (curr == outbox->wndend->next)
				break;		
		if (ackno == (curr->segment->hdr.seqno + ((curr->segment->hdr.plds == 0) ? 1 : curr->segment->hdr.plds))) {
			curr->status = _RUDP_ACKED;
			_slideOutboxWindow(outbox);
			break;
		}
		curr = curr->next;
	}
}

void _slideOutboxWindow(SegmentOutbox *outbox) {
	while (outbox->wndbase) {
		if (outbox->wndbase->status != _RUDP_ACKED)
			break;
		outbox->wndbase = outbox->wndbase->next;
		if (outbox->wndend == outbox->tail) {
			outbox->wndend = NULL;
			outbox->awndsize--;
		} else if (outbox->wndend == NULL) {
			outbox->awndsize--;
		} else {
			outbox->wndend = outbox->wndend->next;
		}
		if (outbox->wndbase != NULL)
			_removeOutboxElement(outbox, outbox->wndbase->prev);
		else
			_removeOutboxElement(outbox, outbox->head);	
	}
}

void _removeOutboxElement(SegmentOutbox *outbox, OutboxElement *elem) {
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

char *outboxToString(const SegmentOutbox outbox) {
	OutboxElement *curr = NULL;
	char *stroutbox = NULL;
	char *strsgm = NULL;

	if (!(stroutbox = malloc(sizeof(char) * (1 + outbox.size * (_RUDP_MAX_SGM_OUTPUT + 5 + 1))))) {
		fprintf(stderr, "Cannot allocate string for outbox to string.\n");
		exit(EXIT_FAILURE);
	}

	stroutbox[0] = '\0';

	curr = outbox.head;
	while (curr) {
		if (curr == outbox.wndbase)
			strcat(stroutbox, "<\t");
		else if (curr == outbox.wndend)
			strcat(stroutbox, ">\t");
		else
			strcat(stroutbox, " \t");
		if (curr->status == _RUDP_ACKED)
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
