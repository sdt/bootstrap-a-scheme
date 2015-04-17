SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:%.c=%.o)

CCFLAGS=-Wall

.PHONY:	all clean

.SUFFIXES: .cpp .o

all: bas

.deps: *.c *.h
	$(CC) $(CCFLAGS) -MM *.c > .deps

bas: $(OBJECTS)
	$(LD) $^ -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o bas

-include .deps
