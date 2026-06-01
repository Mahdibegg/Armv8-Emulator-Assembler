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

void output_write() {
    // TODO
}