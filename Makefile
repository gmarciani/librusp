# RUDP MAKEFILE #

# Compiler

CC = gcc

CFLAGS = -g -Wall -O3


# Sources

SRCDIR = src

PROTOCOLDIR = $(SRCDIR)/protocol

BINDIR = bin

TESTDIR = src/test

TESTPREFIX = test_


# Dependencies

PROTOCOL_LIBS = -lpthread

PROTOCOL_UTILS = $(addprefix $(PROTOCOLDIR)/util/, sockmng.h sockmng.c addrutil.h addrutil.c stringutil.h stringutil.c)

PROTOCOL_CORE = $(addprefix $(PROTOCOLDIR)/core/, rudpcore.h rudpcore.c rudpqueue.h rudpqueue.c rudpsegment.h rudpsegment.c)

PROTOCOL =  $(addprefix $(PROTOCOLDIR)/, rudp.h rudp.c) $(PROTOCOL_CORE) $(PROTOCOL_UTILS) $(PROTOCOL_LIBS)


# Tests

test: setup snd rcv sgm thread

rcv: $(TESTDIR)/rcv.c
	$(CC) $(CFLAGS) $(TESTDIR)/rcv.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

snd: $(TESTDIR)/snd.c
	$(CC) $(CFLAGS) $(TESTDIR)/snd.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

sgm: $(TESTDIR)/sgm.c
	$(CC) $(CFLAGS) $(TESTDIR)/sgm.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

thread: $(TESTDIR)/thread.c
	$(CC) $(CFLAGS) $(TESTDIR)/thread.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

clean-test: 
	rm -frv $(BINDIR)/$(TESTPREFIX)*


# Utility

setup: 
	mkdir -pv $(BINDIR)
