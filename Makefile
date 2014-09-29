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

PROTOCOL_MAILBOX = $(addprefix $(SRCDIR)/core/, rudpoutbox.h rudpoutbox.c rudpinbox.h rudpinbox.c) $(PROTOCOL_SEGMENTS)

PROTOCOL_CONNECTION = $(addprefix $(SRCDIR)/core/, rudpconnection.h rudpconnection.c) $(PROTOCOL_MAILBOX)

PROTOCOL =  $(addprefix $(SRCDIR)/, rudp.h rudp.c) $(PROTOCOL_CONNECTION) 


# Tests

test: setup sender receiver outbox inbox segment thread timer bitmask

sender: $(TESTDIR)/sender.c
	$(CC) $(CFLAGS) $(TESTDIR)/sender.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

receiver: $(TESTDIR)/receiver.c
	$(CC) $(CFLAGS) $(TESTDIR)/receiver.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

outbox: $(TESTDIR)/outbox.c
	$(CC) $(CFLAGS) $(TESTDIR)/outbox.c $(PROTOCOL_MAILBOX) -o $(BINDIR)/$(TESTPREFIX)$@

inbox: $(TESTDIR)/inbox.c
	$(CC) $(CFLAGS) $(TESTDIR)/inbox.c $(PROTOCOL_MAILBOX) -o $(BINDIR)/$(TESTPREFIX)$@

segment: $(TESTDIR)/segment.c
	$(CC) $(CFLAGS) $(TESTDIR)/segment.c $(PROTOCOL_SEGMENTS) -o $(BINDIR)/$(TESTPREFIX)$@

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
