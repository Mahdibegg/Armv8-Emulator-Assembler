#include <stdio.h>
#include <stdlib.h>

#include "binary_writer.h"
#include "shared/bit.h"

/*
 * Writes a 32-bit word to the output file in little-endian byte order
 *
 * f: output file, opened for binary writing
 * word: 32-bit instruction to be written
 */
void write_word(FILE *f, instr_t word) {
    if (f == NULL) {
        fprintf(stderr, "ERROR: File pointer is NULL");
        abort();
    }
}