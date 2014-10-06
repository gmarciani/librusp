# RUDP MAKEFILE #

# Compiler

CC = gcc

CFLAGS = -g -Wall -O1

PFLAGS = -pg


# Sources

SRCDIR = src

BINDIR = bin

COREDIR = $(SRCDIR)/core

UTILDIR = $(SRCDIR)/util

TESTDIR = src/test

TESTPREFIX = test_


# Dependencies

PROTOCOL_LIBS = -lpthread -lrt -lm -lcrypto -lssl

PROTOCOL_UTILS = $(addprefix $(UTILDIR)/, sockutil.h sockutil.c addrutil.h addrutil.c timerutil.h timerutil.c threadutil.h threadutil.c listutil.h listutil.c mathutil.h mathutil.c stringutil.h stringutil.c) $(PROTOCOL_LIBS)

PROTOCOL_SEGMENTS = $(addprefix $(COREDIR)/, rudpsegmentlist.h rudpsegmentlist.c rudpsegment.h rudpsegment.c) $(PROTOCOL_UTILS)

PROTOCOL_MAILBOX = $(addprefix $(COREDIR)/, rudpoutbox.h rudpoutbox.c rudpinbox.h rudpinbox.c) $(PROTOCOL_SEGMENTS)

PROTOCOL_CONNECTION = $(addprefix $(COREDIR)/, rudpconnection.h rudpconnection.c) $(PROTOCOL_MAILBOX)

PROTOCOL =  $(addprefix $(SRCDIR)/, rudp.h rudp.c) $(PROTOCOL_CONNECTION)


# Tests

test: setup sender receiver outbox inbox segmentlist segment timer buffer md5 random 

sender: $(TESTDIR)/sender.c
	$(CC) $(CFLAGS) $(TESTDIR)/sender.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

receiver: $(TESTDIR)/receiver.c
	$(CC) $(CFLAGS) $(TESTDIR)/receiver.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

outbox: $(TESTDIR)/outbox.c
	$(CC) $(CFLAGS) $(TESTDIR)/outbox.c $(COREDIR)/rudpoutbox.h $(COREDIR)/rudpoutbox.c $(PROTOCOL_SEGMENTS) -o $(BINDIR)/$(TESTPREFIX)$@

inbox: $(TESTDIR)/inbox.c
	$(CC) $(CFLAGS) $(TESTDIR)/inbox.c $(COREDIR)/rudpinbox.h $(COREDIR)/rudpinbox.c $(PROTOCOL_SEGMENTS) -o $(BINDIR)/$(TESTPREFIX)$@

segmentlist: $(TESTDIR)/segmentlist.c
	$(CC) $(CFLAGS) $(TESTDIR)/segmentlist.c $(PROTOCOL_SEGMENTS) -o $(BINDIR)/$(TESTPREFIX)$@

segment: $(TESTDIR)/segment.c
	$(CC) $(CFLAGS) $(TESTDIR)/segment.c $(PROTOCOL_SEGMENTS) -o $(BINDIR)/$(TESTPREFIX)$@

timer: $(TESTDIR)/timer.c
	$(CC) $(CFLAGS) $(TESTDIR)/timer.c $(PROTOCOL_UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

buffer: $(TESTDIR)/buffer.c
	$(CC) $(CFLAGS) $(TESTDIR)/buffer.c $(UTILDIR)/stringutil.h $(UTILDIR)/stringutil.c -o $(BINDIR)/$(TESTPREFIX)$@

md5: $(TESTDIR)/md5.c
	$(CC) $(CFLAGS) $(TESTDIR)/md5.c $(UTILDIR)/mathutil.h $(UTILDIR)/mathutil.c -o $(BINDIR)/$(TESTPREFIX)$@ -lm -lssl -lcrypto

random: $(TESTDIR)/random.c
	$(CC) $(CFLAGS) $(TESTDIR)/random.c $(UTILDIR)/mathutil.h $(UTILDIR)/mathutil.c -o $(BINDIR)/$(TESTPREFIX)$@ -lm

clean-test: 
	rm -frv $(BINDIR)/$(TESTPREFIX)*


# Utility

setup: 
	mkdir -pv $(BINDIR)
