# RUDP MAKEFILE #

# Compiler

CC = gcc

CFLAGS = -g -Wall -O3

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

PROTOCOL_UTILS = $(addprefix $(UTILDIR)/, sockutil.h sockutil.c addrutil.h addrutil.c timerutil.h timerutil.c threadutil.h threadutil.c listutil.h listutil.c mathutil.h mathutil.c stringutil.h stringutil.c macroutil.h) $(PROTOCOL_LIBS)

PROTOCOL_SEGMENTS = $(addprefix $(COREDIR)/, rudpsgmbuffer.h rudpsgmbuffer.c rudptsgmbuffer.h rudptsgmbuffer.c rudpsgm.h rudpsgm.c rudpseqn.h rudpseqn.h rudpseqn.c) $(PROTOCOL_UTILS)

PROTOCOL_CONNECTION = $(addprefix $(COREDIR)/, rudpconn.h rudpconn.c rudpconnmng.h rudpconnmng.c rudptimeo.h) $(PROTOCOL_SEGMENTS)

PROTOCOL =  $(addprefix $(SRCDIR)/, rudp.h rudp.c) $(PROTOCOL_CONNECTION)

# Targets

all: createbindir echosnd echorcv tsgmbuffer sgmbuffer sgm timer buffer string math macro

echosnd: $(TESTDIR)/echosnd.c
	$(CC) $(CFLAGS) $(TESTDIR)/echosnd.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

echorcv: $(TESTDIR)/echorcv.c
	$(CC) $(CFLAGS) $(TESTDIR)/echorcv.c $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

tsgmbuffer: $(TESTDIR)/tsgmbuffer.c
	$(CC) $(CFLAGS) $(TESTDIR)/tsgmbuffer.c $(PROTOCOL_SEGMENTS) -o $(BINDIR)/$(TESTPREFIX)$@

sgmbuffer: $(TESTDIR)/sgmbuffer.c
	$(CC) $(CFLAGS) $(TESTDIR)/sgmbuffer.c $(PROTOCOL_SEGMENTS) -o $(BINDIR)/$(TESTPREFIX)$@

sgm: $(TESTDIR)/sgm.c
	$(CC) $(CFLAGS) $(TESTDIR)/sgm.c $(PROTOCOL_SEGMENTS) -o $(BINDIR)/$(TESTPREFIX)$@

timer: $(TESTDIR)/timer.c
	$(CC) $(CFLAGS) $(TESTDIR)/timer.c $(PROTOCOL_UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

buffer: $(TESTDIR)/buffer.c
	$(CC) $(CFLAGS) $(TESTDIR)/buffer.c $(PROTOCOL_UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

string: $(TESTDIR)/string.c
	$(CC) $(CFLAGS) $(TESTDIR)/string.c $(PROTOCOL_UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

math: $(TESTDIR)/math.c
	$(CC) $(CFLAGS) $(TESTDIR)/math.c $(PROTOCOL_UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

macro: $(TESTDIR)/macro.c
	$(CC) $(CFLAGS) $(TESTDIR)/macro.c $(UTILDIR)/macroutil.h -o $(BINDIR)/$(TESTPREFIX)$@

clean: 
	rm -frv $(BINDIR)/$(TESTPREFIX)*

# Utilities

createbindir: 
	mkdir -pv $(BINDIR)
