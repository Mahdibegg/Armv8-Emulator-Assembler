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
	include_emulate/IO/io.o \
	include_emulate/memory/memory.o \
	include_emulate/registers/registers.o \
	include_emulate/pipeline/pipeline.o \
	include_emulate/instruction_types/branch/branch.o \
	include_emulate/instruction_types/data_processing/data_processing.o \
	include_emulate/instruction_types/load_store/load_store.o \
	include_emulate/bit_utils/bit.o
emulate: $(EMULATE_OBJS)
	$(CC) $(CFLAGS) -o emulate $(EMULATE_OBJS)

# CLEAN MAKE CONFIG

CLEAN_OBJS = *.o \
	assemble \
	emulate \
	include_emulate/IO/*.o \
	include_emulate/memory/*.o \
	include_emulate/registers/*.o \
	include_emulate/pipeline/*.o \
	include_emulate/instruction_types/branch/*.o \
	include_emulate/instruction_types/data_processing/*.o \
	include_emulate/instruction_types/load_store/*.o \
	include_emulate/memory_registers_tests/*.o \
	include_emulate/bit_utils/*.o \
	test_suite/*.o \
	test_io \
	test_registers \
	test_memory \
	test_branch \
	test_bit_utils
clean:
	$(RM) $(CLEAN_OBJS)

# TEST MAKE CONFIGS

# test_register unit test make config
TEST_REGISTER_OBJS = test_suite/test_registers.o \
	include_emulate/registers/registers.o
test_registers: $(TEST_REGISTER_OBJS)
	$(CC) $(CFLAGS) -o test_registers $(TEST_REGISTER_OBJS)

# test_memory unit test make config
TEST_MEMORY_OBJS = test_suite/test_memory.o \
	include_emulate/memory/memory.o
test_memory: $(TEST_MEMORY_OBJS)
	$(CC) $(CFLAGS) -o test_memory $(TEST_MEMORY_OBJS)

# branch unit test make config
TEST_BRANCH_OBJS = test_suite/test_branch.o \
	include_emulate/instruction_types/branch/branch.o \
	include_emulate/registers/registers.o \
	include_emulate/bit_utils/bit.o
test_branch: $(TEST_BRANCH_OBJS)
	$(CC) $(CFLAGS) -o test_branch $(TEST_BRANCH_OBJS)

# bit_utils unit test make config
TEST_BIT_UTILS_OBJS = test_suite/test_bit_utils.o \
	include_emulate/bit_utils/bit.o
test_bit_utils: $(TEST_BIT_UTILS_OBJS)
	$(CC) $(CFLAGS) -o test_bit_utils $(TEST_BIT_UTILS_OBJS)

# IO unit test make config
TEST_IO_OBJS = test_suite/test_io.o \
	include_emulate/IO/io.o \
	include_emulate/memory/memory.o \
    include_emulate/registers/registers.o
test_io: $(TEST_IO_OBJS)
	$(CC) $(CFLAGS) -o test_io $(TEST_IO_OBJS)
