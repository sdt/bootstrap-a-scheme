uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

ifeq ($(uname_S),Darwin)
        # Native build on yosemite. Requires: brew install readline
        READLINE=/usr/local/Cellar/readline/6.3.8
        INCPATHS=-I$(READLINE)/include
        LIBPATHS=-L$(READLINE)/lib
endif

SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:%.c=%.o)

CCFLAGS=-Wall -g $(INCPATHS)
LDFLAGS=-g $(LIBPATHS) -lreadline -lhistory

# CCFLAGS+=-O3
# LDFLAGS+=-O3

# Optional debug flags
# CCFLAGS+=-DDEBUG_WIPE_HEAP		# wipe new heap before gc starts
# CCFLAGS+=-DDEBUG_GC_EVERY_ALLOC	# run a gc before *every* allocation

.PHONY:	all clean test depend

.SUFFIXES: .c .o

all: .deps bas

depend: .deps

.deps: *.c *.h Makefile
	$(CC) $(CCFLAGS) -MM *.c > .deps

bas: $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o bas

test: bas
	PERL5LIB=t/lib prove -r t

vtest: bas
	PERL5LIB=t/lib prove -vr t

-include .deps
