#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "assemble/binary_writer.h"

#define FILENAME "test_binary_output.bin"

int main(void) {
    FILE *f = fopen(FILENAME, "wb");
    assert(f != NULL);

    /* Test word: known pattern */
    instr_t word = 0x12345678;

    /* Write word */
    binary_writer(f, word);
    fclose(f);

    /* Reopen file for reading */
    f = fopen(FILENAME, "rb");
    assert(f != NULL);

    /* Read back bytes */
    unsigned char bytes[4];
    size_t read = fread(bytes, sizeof(unsigned char), 4, f);
    assert(read == 4);

    fclose(f);

    /*
     * Little endian expected:
     * 0x12345678 → [0x78, 0x56, 0x34, 0x12]
     */
    assert(bytes[0] == 0x78);
    assert(bytes[1] == 0x56);
    assert(bytes[2] == 0x34);
    assert(bytes[3] == 0x12);

    printf("Binary writer test passed!\n");

    return 0;
}