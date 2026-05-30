#include "instruction_types/bit_utils/bit.h"

word_t extract_bits(instr_t instruction, unsigned low, unsigned high) {

    // validating bit input for internal error
    if (low < 0 || high >= 32 || low > high) {
        fprintf(stderr,"Invalid bit range for extract_bits (data_process): high = %u, low = %u\n", high, low);
        exit(EXIT_FAILURE);
    }

    // rare case: bits 0-31 extracted
    if (high - low == 32) {
        return instruction;
    }

    // width = number of bits to extract to create mask
    unsigned width = high - low + 1;
    word_t mask = ((word_t) 1 << width) - 1;

    // shift instruction to mask position and apply mask
    return (instruction >> low) & mask;
}