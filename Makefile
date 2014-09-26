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

PROTOCOL_CORE = $(addprefix $(SRCDIR)/core/, rudpcore.h rudpcore.c rudpsegment.h rudpsegment.c)

PROTOCOL =  $(addprefix $(SRCDIR)/, rudp.h rudp.c) $(PROTOCOL_CORE) $(PROTOCOL_UTILS) $(PROTOCOL_LIBS)


# Tests

test: setup snd rcv list stream sgm thread timer

rcv: $(TESTDIR)/rcv.c
	$(CC) $(CFLAGS) $(TESTDIR)/rcv.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

snd: $(TESTDIR)/snd.c
	$(CC) $(CFLAGS) $(TESTDIR)/snd.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

list: $(TESTDIR)/list.c
	$(CC) $(CFLAGS) $(TESTDIR)/list.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

stream: $(TESTDIR)/stream.c
	$(CC) $(CFLAGS) $(TESTDIR)/stream.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

sgm: $(TESTDIR)/sgm.c
	$(CC) $(CFLAGS) $(TESTDIR)/sgm.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

thread: $(TESTDIR)/thread.c
	$(CC) $(CFLAGS) $(TESTDIR)/thread.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

timer: $(TESTDIR)/timer.c
	$(CC) $(CFLAGS) $(TESTDIR)/timer.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

clean-test: 
	rm -frv $(BINDIR)/$(TESTPREFIX)*


# Utility

setup: 
	mkdir -pv $(BINDIR)
