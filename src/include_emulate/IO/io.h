#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../types.h"
#include "../memory/memory.h"
#include "../state.h"

// Argument checking + validating correct file extensions
void validate_args(int argc, char **argv, char **input, char **output);

// Select the output stream
FILE *setup_output(char *outputfile);

// Load binary input file into emulated memory
void binary_loader(machine_state_t *state, char *inputfile);

// Write output to given stream
void output_write(machine_state_t *state, FILE *out);

#endif