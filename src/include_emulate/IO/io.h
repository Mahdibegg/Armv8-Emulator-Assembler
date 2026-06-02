#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../types.h"
#include "../memory/memory.h"
#include "../state.h"

// Argument + file handling
void validate_args(int argc, char **argv, char **input, char **output);

// File setup
FILE *setup_output(char *outputfile);

// Binary loader
void binary_loader(machine_state_t *state, char *inputfile);

// Output writer
void output_write(machine_state_t *state, FILE *out);

#endif