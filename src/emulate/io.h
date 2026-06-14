#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emulate/state.h"

/*
 * Argument checking + validating correct file extensions
 * checks the argument count and that the file extensions are valid
 */
void validate_args(int argc, char **argv, char **input, char **output);

/*
 * Select the output stream
 * returns the opened output file, or stdout when none is given
 */
FILE *setup_output(char *outputfile);

/*
 * Load binary input file into emulated memory
 * reads the file bytes into the machine state memory
 */
void binary_loader(machine_state_t *state, char *inputfile);

/*
 * Write output to given stream
 * prints the final register, PC, PSTATE and memory state
 */
void output_write(machine_state_t *state, FILE *out);

#endif