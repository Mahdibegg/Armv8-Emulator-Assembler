#include "bit.h"

#define BIT_MSB_POS_64 63
#define BIT_MSB_POS_32 31

dword_t extract_bits(dword_t dword, unsigned low, unsigned high) {

    // validating bit input for internal error
    if (low < 0 || high >= 64 || low > high) {
        fprintf(stderr,"Invalid bit range for extract_bits (data_process): high = %u, low = %u\n", high, low);
        exit(EXIT_FAILURE);
    }

    // rare case: bits 0-31 extracted
    if (high - low == 64) {
        return dword;
    }

    // width = number of bits to extract to create mask
    unsigned width = high - low + 1;
    dword_t mask = ((dword_t) 1 << width) - 1;

    // shift instruction to mask position and apply mask
    return (dword >> low) & mask;
}

bit_t sign_bit_32(word_t word) {
    return (bit_t) extract_bits(word, BIT_MSB_POS_32, BIT_MSB_POS_32);
}

bit_t sign_bit_64(dword_t dword) {
    return (bit_t) extract_bits(dword, BIT_MSB_POS_64, BIT_MSB_POS_64);
}