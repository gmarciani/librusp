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

PERFORMANCE_TESTDIR = $(TESTDIR)/performance

TESTPREFIX = test_

# Dependencies

LIBS = -pthread -lrt -lm -lcrypto -lssl

UTILS = $(addprefix $(UTILDIR)/, sockutil.h sockutil.c addrutil.h addrutil.c timeutil.h timeutil.c threadutil.h threadutil.c listutil.h listutil.c mathutil.h mathutil.c fileutil.h fileutil.c stringutil.h stringutil.c macroutil.h) $(LIBS)

SEGMENTS = $(addprefix $(SEGMENTDIR)/, sgm.h sgm.c seqn.h seqn.c) $(UTILS)

BUFFERS = $(addprefix $(BUFFERDIR)/, sgmbuff.h sgmbuff.c strbuff.h strbuff.c) $(SEGMENTS)

CONNECTION = $(addprefix $(CONNECTIONDIR)/, conn.h conn.c connmng.h connmng.c timeo.h timeo.c wnd.h wnd.c) $(BUFFERS)

PROTOCOL =  $(addprefix $(SRCDIR)/, rudp.h rudp.c) $(CONNECTION)

# Targets

test: testdir communication buffer segment base performance

testdir: 
	mkdir -pv $(BINDIR)
	
communication: echosnd echorcv filesnd filercv

echosnd: $(COMMUNICATION_TESTDIR)/echosnd.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@ $(PFLAGS)

echorcv: $(COMMUNICATION_TESTDIR)/echorcv.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@
	
filesnd: $(COMMUNICATION_TESTDIR)/filesnd.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@

filercv: $(COMMUNICATION_TESTDIR)/filercv.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$(TESTPREFIX)$@
	
buffer: sgmbuffer strbuffer

sgmbuffer: $(BUFFER_TESTDIR)/sgmbuffer.c
	$(CC) $(CFLAGS) $< $(BUFFERS) -o $(BINDIR)/$(TESTPREFIX)$@
	
strbuffer: $(BUFFER_TESTDIR)/strbuffer.c
	$(CC) $(CFLAGS) $< $(BUFFERS) -o $(BINDIR)/$(TESTPREFIX)$@

segment: $(SEGMENT_TESTDIR)/segment.c
	$(CC) $(CFLAGS) $< $(SEGMENTS) -o $(BINDIR)/$(TESTPREFIX)$@
	
base: sync timer file str rnd macro

sync: $(BASE_TESTDIR)/sync.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

timer: $(BASE_TESTDIR)/timer.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$(TESTPREFIX)$@
	
file: $(BASE_TESTDIR)/file.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

str: $(BASE_TESTDIR)/str.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

rnd: $(BASE_TESTDIR)/rnd.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$(TESTPREFIX)$@

macro: $(BASE_TESTDIR)/macro.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$(TESTPREFIX)$@
	
performance: syncperf baseperf
	
baseperf: $(PERFORMANCE_TESTDIR)/baseperf.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$(TESTPREFIX)$@ $(PFLAGS)

syncperf: $(PERFORMANCE_TESTDIR)/syncperf.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$(TESTPREFIX)$@ $(PFLAGS)
clean-test: 
	rm -frv $(BINDIR)/*