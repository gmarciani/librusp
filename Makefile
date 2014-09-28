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

PROTOCOL_UTILS = $(addprefix $(SRCDIR)/util/, sockmng.h sockmng.c addrutil.h addrutil.c timerutil.h timerutil.c stringutil.h stringutil.c) $(PROTOCOL_LIBS)

PROTOCOL_SEGMENTS = $(addprefix $(SRCDIR)/core/, rudpsegment.h rudpsegment.c) $(PROTOCOL_UTILS)

PROTOCOL_SOCKETS = $(addprefix $(SRCDIR)/core/, rudpoutbox.h rudpoutbox.c rudpinbox.h rudpinbox.c rudptimer.h rudptimer.c) $(PROTOCOL_SEGMENTS)

PROTOCOL_CORE = $(addprefix $(SRCDIR)/core/, rudpcore.h rudpcore.c) $(PROTOCOL_SOCKETS)

PROTOCOL =  $(addprefix $(SRCDIR)/, rudp.h rudp.c) $(PROTOCOL_CORE) 


# Tests

test: setup snd rcv outbox inbox sgm thread timer bitmask

rcv: $(TESTDIR)/rcv.c
	$(CC) $(CFLAGS) $(TESTDIR)/rcv.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

snd: $(TESTDIR)/snd.c
	$(CC) $(CFLAGS) $(TESTDIR)/snd.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

outbox: $(TESTDIR)/outbox.c
	$(CC) $(CFLAGS) $(TESTDIR)/outbox.c $(PROTOCOL_SOCKETS) -o $(BINDIR)/$(TESTPREFIX)$@

inbox: $(TESTDIR)/inbox.c
	$(CC) $(CFLAGS) $(TESTDIR)/inbox.c $(PROTOCOL_SOCKETS) -o $(BINDIR)/$(TESTPREFIX)$@

sgm: $(TESTDIR)/sgm.c
	$(CC) $(CFLAGS) $(TESTDIR)/sgm.c $(PROTOCOL_SEGMENTS) -o $(BINDIR)/$(TESTPREFIX)$@

thread: $(TESTDIR)/thread.c
	$(CC) $(CFLAGS) $(TESTDIR)/thread.c $(PROTOCOL_UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

timer: $(TESTDIR)/timer.c
	$(CC) $(CFLAGS) $(TESTDIR)/timer.c $(PROTOCOL_UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

bitmask: $(TESTDIR)/bitmask.c
	$(CC) $(CFLAGS) $(TESTDIR)/bitmask.c $(PROTOCOL_UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

clean-test: 
	rm -frv $(BINDIR)/$(TESTPREFIX)*


# Utility

setup: 
	mkdir -pv $(BINDIR)
