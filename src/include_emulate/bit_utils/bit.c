#include "bit.h"

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

int64_t sign_extend(dword_t value, unsigned bits) {

    // bit that determines sign = sign bit
    // sign bit is the last bit which is bits -1 
    
    unsigned sign_bit = bits-1;

    // next we check if it is negative(1) or positive(0)
    // make all bits 0 except the sign bit then AND it with the original value, if it is 0(false) then positive, otherwise true for negative


    // this puts 1 in the position of the sign_bit
    dword_t sign = ((dword_t)1) << sign_bit;

    if (value & sign) {
        // if the condition evaluates to true then value is negative so we need to fill upper bits with 1
        // we can make a mask by filling upper bits with 1 and the lower bits with 0 
        dword_t extension = ~((dword_t)0) << bits;

        // then we add the original value bits to the lower bits of the extension 
        value = value | extension;
    }
    // need to return it as signed as final value can be negative
    return (int64_t)value;
}