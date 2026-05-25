CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic\
	-I include_emulate

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: assemble.o
	$(CC) $(CFLAGS) -o assemble assemble.o

emulate: emulate.o include_emulate/memory/memory.o include_emulate/registers/registers.o
	$(CC) $(CFLAGS) -o emulate emulate.o include_emulate/memory/memory.o include_emulate/registers/registers.o

clean:
	$(RM) *.o include_emulate/memory/*.o include_emulate/registers/*.o assemble emulate
	
