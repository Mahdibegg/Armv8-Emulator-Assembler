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

EMULATE_OBJS = emulate.o \
	include_emulate/memory/memory.o \
	include_emulate/registers/registers.o \
	include_emulate/pipeline/pipeline.o \
	include_emulate/instruction_types/branch/branch.o \
	include_emulate/instruction_types/data_processing/data_processing.o \
	include_emulate/instruction_types/load_store/load_store.o \
	include_emulate/bit_utils/bit.o
emulate: $(EMULATE_OBJS)
	$(CC) $(CFLAGS) -o emulate $(EMULATE_OBJS)

CLEAN_OBJS = *.o \
	assemble \
	emulate \
	include_emulate/memory/*.o \
	include_emulate/registers/*.o \
	include_emulate/pipeline/*.o \
	include_emulate/instruction_types/branch/*.o \
	include_emulate/instruction_types/data_processing/*.o \
	include_emulate/instruction_types/load_store/*.o \
	include_emulate/memory_registers_tests/*.o \
	include_emulate/bit_utils/*.o \
	test_suite/*.o \
	test_registers \
	test_memory \
	test_load_store
clean:
	$(RM) $(CLEAN_OBJS)



TEST_REGISTER_OBJS = test_suite/test_registers.o \
	include_emulate/registers/registers.o
test_registers: $(TEST_REGISTER_OBJS)
	$(CC) $(CFLAGS) -o test_registers $(TEST_REGISTER_OBJS)

TEST_MEMORY_OBJS = test_suite/test_memory.o \
	include_emulate/memory/memory.o
test_memory: $(TEST_MEMORY_OBJS)
	$(CC) $(CFLAGS) -o test_memory $(TEST_MEMORY_OBJS)

TEST_LOAD_STORE_OBJS = include_emulate/load_store_tests/test_load_store.o \
	include_emulate/instruction_types/load_store/load_store.o \
	include_emulate/registers/registers.o \
	include_emulate/memory/memory.o \
	include_emulate/bit_utils/bit.o
test_load_store: $(TEST_LOAD_STORE_OBJS)
	$(CC) $(CFLAGS) -o test_load_store $(TEST_LOAD_STORE_OBJS)