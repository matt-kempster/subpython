OBJS=repl.o parse.o alloc.o

CFLAGS=-Wall -g -O0
LDFLAGS=-lm

all: subpython

subpython: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o subpython

clean:
	rm -f *.o subpython

.PHONY: all clean
