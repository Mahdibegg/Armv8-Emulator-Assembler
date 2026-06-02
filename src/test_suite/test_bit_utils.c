#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "bit_utils/bit.h"

static void extract_bits_single_bit_test(void) {

    // 0xA = 10 = 0000 0000 0000 1010
    instr_t instruction = 0xA

    assert(extract_bits(instruction, 0, 0) == 0);
    assert(extract_bits(instruction, 1, 1) == 1);
    assert(extract_bits(instruction, 2, 2) == 0);
    assert(extract_bits(instruction, 3, 3) == 1);

    printf("extract_bits_single_bit_test : PASSED");
}



