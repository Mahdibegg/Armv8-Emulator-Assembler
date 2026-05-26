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

emulate: emulate.o include_emulate/memory/memory.o include_emulate/registers/registers.o include_emulate/pipeline/pipeline.o include_emulate/instruction_types/branch/branch.o include_emulate/instruction_types/data_processing/data_processing.o include_emulate/instruction_types/load_store/load_store.o
	$(CC) $(CFLAGS) -o emulate emulate.o include_emulate/memory/memory.o include_emulate/registers/registers.o include_emulate/pipeline/pipeline.o include_emulate/instruction_types/branch/branch.o include_emulate/instruction_types/data_processing/data_processing.o include_emulate/instruction_types/load_store/load_store.o
	
clean:
	$(RM) *.o include_emulate/memory/*.o include_emulate/registers/*.o include_emulate/pipeline/*.o include_emulate/instruction_types/branch/*.o include_emulate/instruction_types/data_processing/*.o include_emulate/instruction_types/load_store/*.o assemble emulate
	
