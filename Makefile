# RUDP MAKEFILE #

# Compiler

CC = gcc

CFLAGS = -g -Wall -O3

PROGCAP = sudo setcap "cap_sys_chroot=+pe"

ACCESSMODE = sudo chmod 0775


# Sources

SRCDIR = src

BINDIR = bin

TESTDIR = src/test

TESTPREFIX = test_


# Dependencies

PROTOCOL = $(addprefix $(SRCDIR)/, protocol/rudp.h protocol/rudp.c protocol/_rudp.h protocol/_rudp.c)

COMMON = $(addprefix $(SRCDIR)/, common/util.h common/util.c)


# RUDP Installation

install: setup rcv snd

rcv: $(SRCDIR)/rcv.c
	$(CC) $(CFLAGS) $(SRCDIR)/rcv.c $(SERVICE) $(PROTOCOL) $(COMMON) -o $(BINDIR)/$@

snd: $(SRCDIR)/snd.c
	$(CC) $(CFLAGS) $(SRCDIR)/snd.c $(SERVICE) $(PROTOCOL) $(COMMON) -o $(BINDIR)/$@


# RUDP Uninstallation
	
uninstall: 
	rm -fv $(BINDIR)/*


# Tests

test: setup padding

padding: $(TESTDIR)/padding.c
	$(CC) $(CFLAGS) $(TESTDIR)/padding.c $(PROTOCOL) $(COMMON) -o $(BINDIR)/$(TESTPREFIX)$@

clean-test: 
	rm -frv $(BINDIR)/$(TESTPREFIX)*


# Utility

setup: 
	mkdir -pv $(BINDIR)
	$(ACCESSMODE) $(BINDIR)
