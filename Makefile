# RUDP MAKEFILE #

# Compiler

CC = gcc

CFLAGS = -g -Wall -O3


# Sources

SRCDIR = src

BINDIR = bin

COREDIR = $(SRCDIR)/core

UTILDIR = $(SRCDIR)/util

TESTDIR = src/test

TESTPREFIX = test_


# Dependencies

PROTOCOL_LIBS = -lpthread -lm

PROTOCOL_UTILS = $(addprefix $(UTILDIR)/, sockmng.h sockmng.c addrutil.h addrutil.c timerutil.h timerutil.c stringutil.h stringutil.c) $(PROTOCOL_LIBS)

PROTOCOL_SEGMENTS = $(addprefix $(COREDIR)/, rudpsegment.h rudpsegment.c) $(PROTOCOL_UTILS)

PROTOCOL_MAILBOX = $(addprefix $(COREDIR)/, rudpsegmentlist.h rudpsegmentlist.c rudpoutbox.h rudpoutbox.c rudpinbox.h rudpinbox.c) $(PROTOCOL_SEGMENTS)

PROTOCOL_CONNECTION = $(addprefix $(COREDIR)/, rudpconnection.h rudpconnection.c) $(PROTOCOL_MAILBOX)

PROTOCOL =  $(addprefix $(SRCDIR)/, rudp.h rudp.c) $(PROTOCOL_CONNECTION) 


# Tests

test: setup sender receiver outbox inbox segmentlist segment thread timer bitmask

sender: $(TESTDIR)/sender.c
	$(CC) $(CFLAGS) $(TESTDIR)/sender.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

receiver: $(TESTDIR)/receiver.c
	$(CC) $(CFLAGS) $(TESTDIR)/receiver.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

outbox: $(TESTDIR)/outbox.c
	$(CC) $(CFLAGS) $(TESTDIR)/outbox.c $(PROTOCOL_MAILBOX) -o $(BINDIR)/$(TESTPREFIX)$@

inbox: $(TESTDIR)/inbox.c
	$(CC) $(CFLAGS) $(TESTDIR)/inbox.c $(PROTOCOL_MAILBOX) -o $(BINDIR)/$(TESTPREFIX)$@

segmentlist: $(TESTDIR)/segmentlist.c
	$(CC) $(CFLAGS) $(TESTDIR)/segmentlist.c $(PROTOCOL_MAILBOX) -o $(BINDIR)/$(TESTPREFIX)$@

segment: $(TESTDIR)/segment.c
	$(CC) $(CFLAGS) $(TESTDIR)/segment.c $(PROTOCOL_SEGMENTS) -o $(BINDIR)/$(TESTPREFIX)$@

timer: $(TESTDIR)/timer.c
	$(CC) $(CFLAGS) $(TESTDIR)/timer.c $(PROTOCOL_UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

#list: $(TESTDIR)/list.c
#	$(CC) $(CFLAGS) $(TESTDIR)/list.c $(PROTOCOL_UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

thread: $(TESTDIR)/thread.c
	$(CC) $(CFLAGS) $(TESTDIR)/thread.c -lpthread -o $(BINDIR)/$(TESTPREFIX)$@

bitmask: $(TESTDIR)/bitmask.c
	$(CC) $(CFLAGS) $(TESTDIR)/bitmask.c $(UTILDIR)/stringutil.h $(UTILDIR)/stringutil.c -o $(BINDIR)/$(TESTPREFIX)$@

clean-test: 
	rm -frv $(BINDIR)/$(TESTPREFIX)*


# Utility

setup: 
	mkdir -pv $(BINDIR)
