OBJS=repl.o lex.o parse.o alloc.o print.o

CFLAGS=-Wall -g -O0 -pedantic -Wextra
LDFLAGS=-lm

all: subpython

subpython: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o subpython

clean:
	rm -f *.o subpython

.PHONY: all clean
