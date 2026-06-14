#include <stdio.h>
#include <stdlib.h>

#include "binary_writer.h"
#include "shared/bit.h"

/* Number of bytes in a 32-bit word */
#define WORD_BYTES (WORD_BITS / 8)

/*
 * Writes a 32-bit word to the output file in little-endian byte order
 *
 * f: output file, opened for binary writing
 * word: 32-bit instruction to be written
 */
void binary_writer(FILE *f, instr_t word) {
    if (f == NULL) {
        fprintf(stderr, "ERROR: File pointer is NULL");
        abort();
    }

    /* Split the word into its constituent bytes, least significant first */
    byte_t bytes[WORD_BYTES];
    word_to_bytes_le(word, bytes);

    /* Emit the bytes to the output file */
    if (fwrite(bytes, sizeof(byte_t), WORD_BYTES, f) != WORD_BYTES) {
        fprintf(stderr, "ERROR: Failed to write word to file");
        abort();
    }
}