CC      ?= gcc
CFLAGS  ?= -std=c17 -g \
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE \
	-Wall -Werror -pedantic \
	-I src

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate


ASSEMBLE_OBJS = \
	src/assemble/assemble.o \
	src/assemble/symbol_table.o \
	src/shared/bit.o

assemble: $(ASSEMBLE_OBJS)
	$(CC) $(CFLAGS) -o assemble $(ASSEMBLE_OBJS)

EMULATE_OBJS = \
	src/emulate/emulate.o \
	src/emulate/io.o \
	src/emulate/memory.o \
	src/emulate/registers.o \
	src/emulate/pipeline.o \
	src/emulate/branch.o \
	src/emulate/data_processing.o \
	src/emulate/load_store.o \
	src/shared/bit.o

emulate: $(EMULATE_OBJS)
	$(CC) $(CFLAGS) -o emulate $(EMULATE_OBJS)

# CLEAN MAKE CONFIG

CLEAN_OBJS = \
	*.o \
	assemble \
	emulate \
	src/assemble/*.o \
	src/emulate/*.o \
	src/shared/*.o
clean:
	$(RM) $(CLEAN_OBJS)