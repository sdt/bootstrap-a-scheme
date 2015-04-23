uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

ifeq ($(uname_S),Darwin)
        # Native build on yosemite. Requires: brew install readline
        READLINE=/usr/local/Cellar/readline/6.3.8
        INCPATHS=-I$(READLINE)/include
        LIBPATHS=-L$(READLINE)/lib
endif

SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:%.c=%.o)

CCFLAGS=-Wall $(INCPATHS)
LDFLAGS=-ggdb $(LIBPATHS) -lreadline -lhistory

.PHONY:	all clean test

.SUFFIXES: .c .o

all: bas deps

deps: *.c *.h
	$(CC) $(CCFLAGS) -MM *.c > .deps

bas: $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o bas

test: bas
	PERL5LIB=t/lib prove -vr t

-include .deps
