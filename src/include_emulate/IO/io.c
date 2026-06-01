#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//helper function for checking file ending
static int ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) return 0;

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);

    if (lensuffix > lenstr) return 0;
    
    //move along string until reach suffix, and compare
    return strcmp(str + (lenstr - lensuffix), suffix) == 0;
}

//validate arguments passed in
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

FILE *setup_output() {
    return NULL; // TODO
}

void binary_loader() {
    // TODO
}

void output_write() {
    // TODO
}