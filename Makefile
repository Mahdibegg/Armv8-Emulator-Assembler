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
	src/assemble/reader.o \
	src/shared/bit.o \
	src/shared/file_utils.o

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
	src/shared/bit.o \
	src/shared/file_utils.o

emulate: $(EMULATE_OBJS)
	$(CC) $(CFLAGS) -o emulate $(EMULATE_OBJS)

AT_READ_LINE_OBJS = \
    test/at_read_line.o \
    src/assemble/reader.o \

at_read_line: $(AT_READ_LINE_OBJS)
	$(CC) $(CFLAGS) -o at_read_line $(AT_READ_LINE_OBJS)

AT_BINARY_WRITER_OBJS = \
    test/at_binary_writer.o \
    src/assemble/binary_writer.o \
    src/shared/bit.o

at_binary_writer: $(AT_BINARY_WRITER_OBJS)
	$(CC) $(CFLAGS) -o at_binary_writer $(AT_BINARY_WRITER_OBJS)

TEST_BINS = at_read_line at_binary_writer

test: $(TEST_BINS)

# CLEAN MAKE CONFIG

clean:
	find src -name '*.o' -delete
	find test -name '*.o' -delete
	$(RM) assemble emulate $(TEST_BINS) test_input.txt test_binary_output.bin