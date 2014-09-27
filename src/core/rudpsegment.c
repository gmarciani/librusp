#include "rudpsegment.h"

/* SEGMENT */

const static size_t _RUDP_HDR_FIELDS_SIZE[_RUDP_HDR_FIELDS] = {2, 2, 5, 10, 10};

Segment createSegment(const unsigned short ctrl, unsigned long seqno, unsigned long ackno, const char *pld) {
	Segment sgm;
	size_t pldsize = 0;
	int i;

	sgm.hdr.vers = _RUDP_VERSION;
	sgm.hdr.ctrl = ctrl;
	sgm.hdr.seqno = seqno;
	sgm.hdr.ackno = ackno;

	if (pld) {
		pldsize = (strlen(pld) < _RUDP_MAX_PLD) ? strlen(pld) : _RUDP_MAX_PLD;
		for (i = 0; i < pldsize; i++)
			sgm.pld[i] = pld[i];
	}

	sgm.pld[pldsize] = '\0';

	sgm.hdr.plds = pldsize;

	return sgm;	
}

Segment deserializeSegment(const char *ssgm) {
	Segment sgm;
	char *hdr = NULL;
	char **hdrf = NULL;
	size_t pldsize;
	int i;

	hdr = stringNDuplication(ssgm, _RUDP_MAX_HDR);

	hdrf = splitStringBySection(hdr, _RUDP_HDR_FIELDS_SIZE, _RUDP_HDR_FIELDS);

	sgm.hdr.vers = (unsigned short) atoi(hdrf[0]);
	sgm.hdr.ctrl = (unsigned short) atoi(hdrf[1]);
	sgm.hdr.plds = (unsigned short) atoi(hdrf[2]);
	sgm.hdr.seqno = strtoul(hdrf[3], NULL, 10);
	sgm.hdr.ackno = strtoul(hdrf[4], NULL, 10);

	pldsize = (sgm.hdr.plds < _RUDP_MAX_PLD) ? sgm.hdr.plds : _RUDP_MAX_PLD;

	for (i = 0; i < pldsize; i++)
		sgm.pld[i] = *(ssgm + _RUDP_MAX_HDR + i);

	sgm.pld[i] = '\0';	

	for (i = 0; i < _RUDP_HDR_FIELDS; i++)
		free(hdrf[i]);
	free(hdrf);
	free(hdr);

	return sgm;
}

char *serializeSegment(const Segment sgm) {
	char *ssgm = NULL;

	if (!(ssgm = malloc(sizeof(char) * (_RUDP_MAX_SGM + 1)))) {
		fprintf(stderr, "Error in serialized segment allocation.\n");
		exit(EXIT_FAILURE);
	}

	sprintf(ssgm, "%02hu%02hu%05hu%010lu%010lu%s", sgm.hdr.vers, sgm.hdr.ctrl, sgm.hdr.plds, sgm.hdr.seqno, sgm.hdr.ackno, sgm.pld);

	return ssgm;
}

char *segmentToString(const Segment sgm) {
	char *str = NULL;

	if (!(str = malloc(sizeof(char) * (_RUDP_MAX_SGM_OUTPUT + 1)))) {
		fprintf(stderr, "Error in segment to string allocation.\n");
		exit(EXIT_FAILURE);
	}

	sprintf(str, "vers:%hu ctrl:%hu plds:%hu seqno:%lu ackno:%lu pld:%s", sgm.hdr.vers, sgm.hdr.ctrl, sgm.hdr.plds, sgm.hdr.seqno, sgm.hdr.ackno, sgm.pld);

	return str;
}

void printInSegment(const struct sockaddr_in sndaddr, const Segment sgm) {
	char *time;
	char *addr = NULL;
	char *strsgm = NULL;
	int port;

	time = getTime();
	addr = getIp(sndaddr);
	port = getPort(sndaddr);
	strsgm = segmentToString(sgm);

	printf("[<- SGM] %s src: %s:%d %s\n", time, addr, port, strsgm);

	free(time);
	free(addr);
	free(strsgm);
}

void printOutSegment(const struct sockaddr_in rcvaddr, const Segment sgm) {
	char *time;
	char *addr = NULL;
	char *strsgm = NULL;
	int port;

	time = getTime();
	addr = getIp(rcvaddr);
	port = getPort(rcvaddr);
	strsgm = segmentToString(sgm);
	
	printf("[SGM ->] %s dst: %s:%d %s\n", time, addr, port, strsgm);

	free(time);
	free(addr);
	free(strsgm);
}

/* STREAM */

Stream createStream(const char *msg) {
	Stream stream;
	char **chunks = NULL;
	size_t chunksize;
	int numchunks = 0;
	int i, j;

	stream.segments = NULL;
	stream.size = 0;
	stream.len = 0;

	chunks = splitStringBySize(msg, _RUDP_MAX_PLD, &numchunks);

	if (!(stream.segments = malloc(sizeof(Segment) * numchunks))) {
		fprintf(stderr, "Error in segment stream allocation.\n");
		exit(EXIT_FAILURE);
	}	

	for (i = 0; i < numchunks; i++) {
		stream.segments[i].hdr.vers = _RUDP_VERSION;
		stream.segments[i].hdr.ctrl = _RUDP_DAT;
		stream.segments[i].hdr.seqno = 0;
		stream.segments[i].hdr.ackno = 0;
		chunksize = strlen(chunks[i]);
		for (j = 0; j < chunksize; j++) {
			stream.segments[i].hdr.plds++;
			stream.segments[i].pld[j] = chunks[i][j];
			stream.len++;
		}
		stream.segments[i].pld[j] = '\0';
		stream.size++;
	}

	stream.segments[i - 1].hdr.ctrl = _RUDP_EOS;

	for (i = 0; i < numchunks; i++) 
		free(chunks[i]);
	free(chunks);

	return stream;
}

void freeStream(Stream *stream) {
	if (stream->segments)
		free(stream->segments);		
	stream->size = 0;
	stream->len = 0;
}

/* LIST OF SEGMENTS */

SegmentList createSegmentList(const unsigned long isn, const unsigned long wndsize) {
	SegmentList list;

	if (wndsize == 0) {
		fprintf(stderr, "Cannot create segment list with window size to zero.\n");
		exit(EXIT_FAILURE);
	}

	list.head = NULL;
	list.tail = NULL;
	list.size = 0;
	list.wndbase = NULL;
	list.wndend = NULL;
	list.wndsize = wndsize;	
	list.awndsize = 0;	
	list.nextseqno = isn;	

	return list;
}

void freeSegmentList(SegmentList *list) {
	Element *curr = NULL;

	curr = list->head;
	while (curr) {
		if (curr->prev) {
			free(curr->prev->segment);
			free(curr->prev);
		}
		if (curr == list->tail) {
			free(curr->segment);
			free(curr);
			break;
		}
		curr = curr->next;
	}

	list->head = NULL;
	list->tail = NULL;	
	list->size = 0;
	list->wndbase = NULL;
	list->wndend = NULL;	
	list->wndsize = 0;
	list->awndsize = 0;	
	list->nextseqno = 0;
}

void submitSegment(SegmentList *list, const Segment sgm) {
	Element *new = NULL;

	if (!(new = malloc(sizeof(Element)))) {
		fprintf(stderr, "Cannot allocate memory for element insertion.\n");
		exit(EXIT_FAILURE);
	}	

	if (!(new->segment = malloc(sizeof(Segment)))) {
		fprintf(stderr, "Cannot allocate memory for segment insertion.\n");
		exit(EXIT_FAILURE);
	}

	new->status = _RUDP_UNACKED;	

	new->segment = memcpy(new->segment, &sgm, sizeof(Segment));
	new->segment->hdr.seqno = list->nextseqno;
	list->nextseqno += (new->segment->hdr.plds == 0) ? 1 : new->segment->hdr.plds;

	if (list->size == 0) {
		new->prev = NULL;
		new->next = NULL;
		list->head = new;
		list->tail = new;
		list->wndbase = new;
		list->wndend = new;
		list->awndsize++;
	} else {
		new->prev = list->tail;
		new->next = NULL;
		list->tail->next = new;
		list->tail = new;
		if (list->awndsize < list->wndsize) {
			list->wndend = new;
			list->awndsize++;
		}			
	}	
	
	list->size++;	
}

void submitAck(SegmentList *list, const unsigned long ackno) {
	Element *curr;

	curr = list->wndbase;
	while (curr) {
		if (list->wndend)
			if (curr == list->wndend->next)
				break;		
		if (ackno == (curr->segment->hdr.seqno + ((curr->segment->hdr.plds == 0) ? 1 : curr->segment->hdr.plds))) {
			curr->status = _RUDP_ACKED;
			_slideWindow(list);
			break;
		}
		curr = curr->next;
	}
}

void _slideWindow(SegmentList *list) {
	while (list->wndbase) {
		if (list->wndbase->status != _RUDP_ACKED)
			break;
		list->wndbase = list->wndbase->next;
		if (list->wndend == list->tail) {
			list->wndend = NULL;
			list->awndsize--;
		} else if (list->wndend == NULL) {
			list->awndsize--;
		} else {
			list->wndend = list->wndend->next;
		}
		if (list->wndbase != NULL)
			_removeElement(list, list->wndbase->prev);
		else
			_removeElement(list, list->head);	
	}
}

void _removeElement(SegmentList *list, Element *elem) {
	if (!elem)
		return;

	if ((list->head == elem) && (list->tail == elem)) {
		list->head = NULL;
		list->tail = NULL;				
	} else if (list->head == elem) {
		list->head = elem->next;
		list->head->prev = NULL;
	} else if (list->tail == elem) {
		list->tail = elem->prev;
		list->tail->next = NULL;
	} else {
		elem->prev->next = elem->next;
		elem->next->prev = elem->prev;		
	}
	free(elem->segment);
	free(elem);
	list->size--;
}

char *listToString(const SegmentList list) {
	Element *curr = NULL;
	char *strlist = NULL;
	char *strsgm = NULL;

	if (!(strlist = malloc(sizeof(char) * (1 + list.size * (_RUDP_MAX_SGM_OUTPUT + 5 + 1))))) {
		fprintf(stderr, "Cannot allocate string for list to string.\n");
		exit(EXIT_FAILURE);
	}

	strlist[0] = '\0';

	curr = list.head;
	while (curr) {
		if (curr == list.wndbase)
			strcat(strlist, "<\t");
		else if (curr == list.wndend)
			strcat(strlist, ">\t");
		else
			strcat(strlist, " \t");
		if (curr->status == _RUDP_ACKED)
			strcat(strlist, "A ");
		else
			strcat(strlist, "U ");
		strsgm = segmentToString(*(curr->segment));
		strcat(strlist, strsgm);
		strcat(strlist, "\n");
		curr = curr->next;
		free(strsgm);
	}

	return strlist;
}
