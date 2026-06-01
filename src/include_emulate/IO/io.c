#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function for checking file ending
static int ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) return 0;

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);

    if (lensuffix > lenstr) return 0;
    
    // Move along string until reach where suffix should be, and compare
    return strcmp(str + (lenstr - lensuffix), suffix) == 0;
}

// Validate arguments passed in
void validate_args(int argc, char **argv, char **input, char **output) {

    // Check number of arguments
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: ./emulate <input.bin> OR ./emulate <input.bin> <output.out>\n");
        exit(EXIT_FAILURE);
    }

    // Assign input file
    *input = argv[1];

    // Assign optional output file
    if (argc == 3) {
        *output = argv[2];
    } else {
        *output = NULL;
    }

    // Validate input file extension
    if (!ends_with(*input, ".bin")) {
        fprintf(stderr, "Error: input file must have .bin extension\n");
        exit(EXIT_FAILURE);
    }

    // Validate output file extension (if not provided short circuit)
    if (*output && !ends_with(*output, ".out")) {
        fprintf(stderr, "Error: output file must have .out extension\n");
        exit(EXIT_FAILURE);
    }
}

// Select whether to output to stdout or to an output file
FILE *setup_output(char *outputfile) {

    // If no output file is provided -> use stdout
    if (outputfile == NULL) {
        return stdout;
    }

    // Try to open the output file
    FILE *out = fopen(outputfile, "w");

    // Fail if unsuccessful
    if (out == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    // Use if successful
    return out;
}

// Load the binary input file into state
void binary_loader(machine_state_t *state, char *inputfile) {

    // Try to open the input file
    FILE *file = fopen(inputfile, "rb");

    // Fail if unsuccesful
    if (file == NULL) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    // Read bytes into the memory array, capped at memory size
    size_t bytes_read = fread(state->memory.memory, sizeof(byte_t), MEMORY_SIZE, file);
    
    // Closed file after reading
    fclose(file);

    // Fail if nothing was read to state
    if (bytes_read == 0) {
        fprintf(stderr, "Error: failed to read input file\n");
        exit(EXIT_FAILURE);
    }
}

// Write to given output stream
void output_write(machine_state_t *state, FILE *out) {

    // Registers
    fprintf(out, "Registers :\n");

    // Format register as X[2 digit number], contents as 16 digit hex value padded with 0's
    for (int i = 0; i < 31; i++) {
        fprintf(out, 
            "X%02d = %016lx\n", i, state->general_registers.r[i]
        );
    }

    // PC, formatted as 16 digit hex value padded with 0's
    fprintf(out, 
        "PC = %016lx\n",
        state->special_registers.pc
    );

    // PSTATE, formatted as combination of 4 characters
    fprintf(out, 
        "PSTATE : %c%c%c%c\n",

        state->special_registers.psr.n_flag ? 'N' : '-',  // Negative flag
        state->special_registers.psr.z_flag ? 'Z' : '-',  // Zero flag
        state->special_registers.psr.c_flag ? 'C' : '-',  // Carry flag
        state->special_registers.psr.v_flag ? 'V' : '-'   // Overflow flag
    );

    // Memory, address and value formatted as 8 digit hex values
    fprintf(out, "Non-zero memory:\n");

    for (addr_t addr = 0; addr < MEMORY_SIZE; addr += 4) {

        word_t value = read_word(&state->memory, addr);

        if (value != 0) {
            fprintf(out, 
                "0x%08x: 0x%08x\n", addr, value
            );
        }
    }
}