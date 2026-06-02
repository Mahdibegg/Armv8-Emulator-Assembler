#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "bit_utils/bit.h"

static void extract_bits_single_bit_test(void) {

    // 0xA = 0000 0000 0000 0000 0000 0000 0000 1010
    instr_t instruction = 0xA;

    assert(extract_bits(instruction, 0, 0) == 0);
    assert(extract_bits(instruction, 1, 1) == 1);
    assert(extract_bits(instruction, 2, 2) == 0);
    assert(extract_bits(instruction, 3, 3) == 1);

    printf("extract_bits_single_bit_test: PASSED");
}

static void extract_bits_range_test(void) {
 
    // OxFC = 0000 0000 0000 0000 0000 0000 1111 1100
    instr_t instruction = 0xFC;

    assert(extract_bits(instruction, 0, 3) == 0xC);
    assert(extract_bits(instruction, 4, 7) == 0xF);
    // bits 2-5  1111 = 0xF
    assert(extract_bits(instruction, 2, 5) == 0xF);

    printf("extract_bits_range_test: PASSED");
}

static void sign_extend_positive_values_test(void) {
    
}



