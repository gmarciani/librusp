# RUDP MAKEFILE #

# Compiler

CC = gcc

CFLAGS = -g -Wall -O3


# Sources

SRCDIR = src

BINDIR = bin

TESTDIR = src/test

TESTPREFIX = test_


# Dependencies

PROTOCOL_LIBS = -lpthread -lm

PROTOCOL_UTILS = $(addprefix $(SRCDIR)/util/, sockmng.h sockmng.c addrutil.h addrutil.c timerutil.h timerutil.c stringutil.h stringutil.c)

PROTOCOL_CORE = $(addprefix $(SRCDIR)/core/, rudpcore.h rudpcore.c rudpoutbox.h rudpoutbox.c rudpinbox.h rudpinbox.c rudpsegment.h rudpsegment.c rudptimer.h rudptimer.c)

PROTOCOL =  $(addprefix $(SRCDIR)/, rudp.h rudp.c) $(PROTOCOL_CORE) $(PROTOCOL_UTILS) $(PROTOCOL_LIBS)


# Tests

test: setup snd rcv outbox inbox stream sgm thread timer struct

rcv: $(TESTDIR)/rcv.c
	$(CC) $(CFLAGS) $(TESTDIR)/rcv.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

snd: $(TESTDIR)/snd.c
	$(CC) $(CFLAGS) $(TESTDIR)/snd.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

outbox: $(TESTDIR)/outbox.c
	$(CC) $(CFLAGS) $(TESTDIR)/outbox.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

inbox: $(TESTDIR)/inbox.c
	$(CC) $(CFLAGS) $(TESTDIR)/inbox.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

stream: $(TESTDIR)/stream.c
	$(CC) $(CFLAGS) $(TESTDIR)/stream.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

sgm: $(TESTDIR)/sgm.c
	$(CC) $(CFLAGS) $(TESTDIR)/sgm.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

thread: $(TESTDIR)/thread.c
	$(CC) $(CFLAGS) $(TESTDIR)/thread.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

timer: $(TESTDIR)/timer.c
	$(CC) $(CFLAGS) $(TESTDIR)/timer.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

struct: $(TESTDIR)/struct.c
	$(CC) $(CFLAGS) $(TESTDIR)/struct.c -o $(BINDIR)/$(TESTPREFIX)$@

clean-test: 
	rm -frv $(BINDIR)/$(TESTPREFIX)*


# Utility

setup: 
	mkdir -pv $(BINDIR)
