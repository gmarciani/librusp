# RUDP MAKEFILE #

# Compiler

CC = gcc

CFLAGS = -g -Wall -O3

PFLAGS = -pg

# Bin Directories

BINDIR = bin

# Source Directories

SRCDIR = src

COREDIR = $(SRCDIR)/core

BUFFERDIR = $(COREDIR)/buffer

CONNECTIONDIR = $(COREDIR)/connection

SEGMENTDIR = $(COREDIR)/segment

UTILDIR = $(SRCDIR)/util

# Tests Directory

TESTDIR = src/test

BASE_TESTDIR = $(TESTDIR)/base

BUFFER_TESTDIR = $(TESTDIR)/buffer

COMMUNICATION_TESTDIR = $(TESTDIR)/communication

SEGMENT_TESTDIR = $(TESTDIR)/segment

# Dependencies

LIBS = -lpthread -lrt -lm -lcrypto -lssl

UTILS = $(addprefix $(UTILDIR)/, sockutil.h sockutil.c addrutil.h addrutil.c timerutil.h timerutil.c threadutil.h threadutil.c listutil.h listutil.c mathutil.h mathutil.c stringutil.h stringutil.c macroutil.h) $(LIBS)

SEGMENTS = $(addprefix $(SEGMENTDIR)/, sgm.h sgm.c seqn.h seqn.c) $(UTILS)

BUFFERS = $(addprefix $(BUFFERDIR)/, timeosgmbuff.h timeosgmbuff.c sortsgmbuff.h sortsgmbuff.c strbuff.h strbuff.c) $(SEGMENTS)

CONNECTION = $(addprefix $(CONNECTIONDIR)/, conn.h conn.c connmng.h connmng.c timeo.h) $(BUFFERS)

PROTOCOL =  $(addprefix $(SRCDIR)/, rudp.h rudp.c) $(CONNECTION)

# Targets

test: testdir communication buffer segment base

testdir: 
	mkdir -pv $(BINDIR)
	
communication: echosnd echorcv filesnd filercv

echosnd: $(COMMUNICATION_TESTDIR)/echosnd.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@

echorcv: $(COMMUNICATION_TESTDIR)/echorcv.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@
	
filesnd: $(COMMUNICATION_TESTDIR)/filesnd.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@

filercv: $(COMMUNICATION_TESTDIR)/filercv.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@
	
buffer: sortsgmbuffer strbuffer

sortsgmbuffer: $(BUFFER_TESTDIR)/sortsgmbuffer.c
	$(CC) $(CFLAGS) $< $(BUFFERS) -o $(BINDIR)/$@
	
strbuffer: $(BUFFER_TESTDIR)/strbuffer.c
	$(CC) $(CFLAGS) $< $(BUFFERS) -o $(BINDIR)/$@

segment: $(SEGMENT_TESTDIR)/sgm.c
	$(CC) $(CFLAGS) $< $(SEGMENTS) -o $(BINDIR)/$@
	
base: timer str math macro

timer: $(BASE_TESTDIR)/timer.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$@

str: $(BASE_TESTDIR)/str.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$@

math: $(BASE_TESTDIR)/math.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$@

macro: $(BASE_TESTDIR)/macro.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$@

clean-test: 
	rm -frv $(BINDIR)/*