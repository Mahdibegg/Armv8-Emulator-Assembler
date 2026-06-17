#ifndef BINARY_WRITER_H
#define BINARY_WRITER_H

#include <stdio.h>
#include "shared/types.h"

/*
 * Writes a 32-bit word to the output file in little-endian byte order
 *
 * f: output file, opened for binary writing
 * word: 32-bit instruction to be written
 */
void binary_writer(FILE* f, instr_t word);

#endif