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

FTP = $(addprefix $(SAMPLEDIR)/ftp/, ftpcore.h ftpcore.c) $(PROTOCOL)

# Targets

.PHONY: all clean

all: createdir samples

createdir: 
	@echo "@ Creating Binaries Directory"
	@mkdir -pv $(BINDIR)
	
samples: echos echoc fstores fstorec ftps ftpc samplegen 

echos: $(SAMPLEDIR)/echos.c
	@echo "@ Compiling ECHO Server"
	$(CC) $(CFLAGS) $< $(PROTOCOL) -Isrc -o $(BINDIR)/$@

echoc: $(SAMPLEDIR)/echoc.c
	@echo "@ Compiling ECHO Client"
	$(CC) $(CFLAGS) $< $(PROTOCOL) -Isrc -o $(BINDIR)/$@
	
fstores: $(SAMPLEDIR)/fstores.c
	@echo "@ Compiling FILE STORE Server"
	$(CC) $(CFLAGS) $< $(PROTOCOL) -Isrc -o $(BINDIR)/$@

fstorec: $(SAMPLEDIR)/fstorec.c
	@echo "@ Compiling FILE STORE Client"
	$(CC) $(CFLAGS) $< $(PROTOCOL) -Isrc -o $(BINDIR)/$@
	
ftps: $(SAMPLEDIR)/ftp/ftps.c
	@echo "@ Compiling FTP Server"
	$(CC) $(CFLAGS) $< $(FTP) -Isrc -o $(BINDIR)/$@

ftpc: $(SAMPLEDIR)/ftp/ftpc.c
	@echo "@ Compiling FTP Client"
	$(CC) $(CFLAGS) $< $(FTP) -Isrc -o $(BINDIR)/$@
	
samplegen: $(SAMPLEDIR)/samplegen.c
	@echo "@ Compiling Sample File Generator"
	$(CC) $(CFLAGS) $< $(UTILS) -Isrc/util -o $(BINDIR)/$@
	
clean: 
	@echo "@ Removing Binaries"
	@rm -frv $(BINDIR)/*