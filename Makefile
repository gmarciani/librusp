# RUSP MAKEFILE #

# Compiler

CC = gcc

CFLAGS = -g -Wall -O3

# Directories

SRCDIR = src

SAMPLEDIR = samples

BINDIR = bin

REPODIR = repo

# Dependencies

LIBS = -pthread -lrt -lm -lcrypto -lssl

UTILS = $(addprefix $(SRCDIR)/util/, cliutil.h cliutil.c sockutil.h sockutil.c addrutil.h addrutil.c timeutil.h timeutil.c threadutil.h threadutil.c listutil.h listutil.c mathutil.h mathutil.c fileutil.h fileutil.c stringutil.h stringutil.c macroutil.h) $(LIBS)

SEGMENTS = $(addprefix $(SRCDIR)/core/segment/, sgm.h sgm.c seqn.h seqn.c) $(UTILS)

BUFFERS = $(addprefix $(SRCDIR)/core/buffer/, sgmbuff.h sgmbuff.c strbuff.h strbuff.c) $(SEGMENTS)

CONNECTION = $(addprefix $(SRCDIR)/core/connection/, conn.h conn.c timeo.h timeo.c wnd.h wnd.c) $(BUFFERS)

PROTOCOL =  $(addprefix $(SRCDIR)/, rusp.h rusp.c) $(CONNECTION)

LFTP = $(addprefix $(SAMPLEDIR)/lftp/, lftpcore.h lftpcore.c) $(PROTOCOL)

# Targets

.PHONY: all echo upload lftp clean

all: createdir samples

createdir: 
	@echo "@ Creating Binaries Directory"
	@mkdir -pv $(BINDIR)
	
samples: echo upload lftp samplegen 

echo: echos echoc

upload: ups upc

lftp: lftps lftpc

echos: $(SAMPLEDIR)/echo/echos.c
	@echo "@ Compiling ECHO Server"
	$(CC) $(CFLAGS) $< $(PROTOCOL) -Isrc -o $(BINDIR)/$@

echoc: $(SAMPLEDIR)/echo/echoc.c
	@echo "@ Compiling ECHO Client"
	$(CC) $(CFLAGS) $< $(PROTOCOL) -Isrc -o $(BINDIR)/$@
	
ups: $(SAMPLEDIR)/upload/ups.c
	@echo "@ Compiling UPLOAD Server"
	$(CC) $(CFLAGS) $< $(PROTOCOL) -Isrc -o $(BINDIR)/$@

upc: $(SAMPLEDIR)/upload/upc.c
	@echo "@ Compiling UPLOAD Client"
	$(CC) $(CFLAGS) $< $(PROTOCOL) -Isrc -o $(BINDIR)/$@
	
lftps: $(SAMPLEDIR)/lftp/lftps.c
	@echo "@ Compiling LFTP Server"
	$(CC) $(CFLAGS) $< $(LFTP) -Isrc -o $(BINDIR)/$@

lftpc: $(SAMPLEDIR)/lftp/lftpc.c
	@echo "@ Compiling LFTP Client"
	$(CC) $(CFLAGS) $< $(LFTP) -Isrc -o $(BINDIR)/$@
	
samplegen: $(SAMPLEDIR)/samplegen.c
	@echo "@ Compiling Sample File Generator"
	$(CC) $(CFLAGS) $< $(UTILS) -Isrc/util -o $(BINDIR)/$@
	
clean: 
	@echo "@ Removing Binaries"
	@rm -frv $(BINDIR)/*