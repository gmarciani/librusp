# RUSP MAKEFILE #

# Compiler

CC = gcc

CFLAGS = -g -Wall -O3

PFLAGS = -pg

# Bin Directories

BINDIR = bin

SAMPLEDIR = $(BINDIR)/sample

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

# Dependencies

LIBS = -pthread -lrt -lm -lcrypto -lssl

UTILS = $(addprefix $(UTILDIR)/, cliutil.h cliutil.c sockutil.h sockutil.c addrutil.h addrutil.c timeutil.h timeutil.c threadutil.h threadutil.c listutil.h listutil.c mathutil.h mathutil.c fileutil.h fileutil.c stringutil.h stringutil.c macroutil.h) $(LIBS)

SEGMENTS = $(addprefix $(SEGMENTDIR)/, sgm.h sgm.c seqn.h seqn.c) $(UTILS)

BUFFERS = $(addprefix $(BUFFERDIR)/, sgmbuff.h sgmbuff.c strbuff.h strbuff.c) $(SEGMENTS)

CONNECTION = $(addprefix $(CONNECTIONDIR)/, conn.h conn.c timeo.h timeo.c wnd.h wnd.c) $(BUFFERS)

PROTOCOL =  $(addprefix $(SRCDIR)/, rusp.h rusp.c) $(CONNECTION)

# Targets

all: testdir communication filegen buffer segment base performance

testdir: 
	mkdir -pv $(BINDIR)
	mkdir -pv $(SAMPLEDIR)
	
communication: snd rcv echosnd echorcv filesnd filercv filesnd_tcp filercv_tcp

snd: $(COMMUNICATION_TESTDIR)/snd.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@

rcv: $(COMMUNICATION_TESTDIR)/rcv.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@

echosnd: $(COMMUNICATION_TESTDIR)/echosnd.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@

echorcv: $(COMMUNICATION_TESTDIR)/echorcv.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@
	
filesnd: $(COMMUNICATION_TESTDIR)/filesnd.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@

filercv: $(COMMUNICATION_TESTDIR)/filercv.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@
	
filesnd_tcp: $(COMMUNICATION_TESTDIR)/filesnd_tcp.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@

filercv_tcp: $(COMMUNICATION_TESTDIR)/filercv_tcp.c
	$(CC) $(CFLAGS) $< $(PROTOCOL) -o $(BINDIR)/$@
	
filegen: $(BASE_TESTDIR)/filegen.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(SAMPLEDIR)/$@
	
buffer: sgmbuffer strbuffer

sgmbuffer: $(BUFFER_TESTDIR)/sgmbuffer.c
	$(CC) $(CFLAGS) $< $(BUFFERS) -o $(BINDIR)/$@
	
strbuffer: $(BUFFER_TESTDIR)/strbuffer.c
	$(CC) $(CFLAGS) $< $(BUFFERS) -o $(BINDIR)/$@

segment: $(SEGMENT_TESTDIR)/segment.c
	$(CC) $(CFLAGS) $< $(SEGMENTS) -o $(BINDIR)/$@
	
base: sync timer str rnd macro

sync: $(BASE_TESTDIR)/sync.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$@

timer: $(BASE_TESTDIR)/timer.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$@

str: $(BASE_TESTDIR)/str.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$@

rnd: $(BASE_TESTDIR)/rnd.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$@

macro: $(BASE_TESTDIR)/macro.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$@
	
performance: syncperf baseperf
	
baseperf: $(PERFORMANCE_TESTDIR)/baseperf.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$@

syncperf: $(PERFORMANCE_TESTDIR)/syncperf.c
	$(CC) $(CFLAGS) $< $(UTILS) -o $(BINDIR)/$@
	
clean: 
	rm -frv $(BINDIR)/*