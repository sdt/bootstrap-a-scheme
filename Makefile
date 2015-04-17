SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:%.c=%.o)

CCFLAGS=-Wall

.PHONY:	all clean

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

-include .deps
