#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "bit_utils/bit.h"

/*
    2 functions that test extract_bits 
    extract_bits_single_bit_test - tests if each bit is extracted correctly
    extract_bits_range_test - tests if the range of bits extracted correctly
*/
static void extract_bits_single_bit_test(void) {

    // 0xA = 0000 0000 0000 0000 0000 0000 0000 1010
    instr_t instruction = 0xA;

    assert(extract_bits(instruction, 0, 0) == 0);
    assert(extract_bits(instruction, 1, 1) == 1);
    assert(extract_bits(instruction, 2, 2) == 0);
    assert(extract_bits(instruction, 3, 3) == 1);

    printf("extract_bits_single_bit_test: PASSED\n");
}

static void extract_bits_range_test(void) {
 
    // OxFC = 0000 0000 0000 0000 0000 0000 1111 1100
    instr_t instruction = 0xFC;

    assert(extract_bits(instruction, 0, 3) == 0xC);
    assert(extract_bits(instruction, 4, 7) == 0xF);
    // bits 2-5  1111 = 0xF
    assert(extract_bits(instruction, 2, 5) == 0xF);

    printf("extract_bits_range_test: PASSED\n");
}

/*
    2 functions that test sign_extend
    sign_extend_positive_values_test - tests if positive values are sign_extended to 64 bits correctly
    sign_extend_negative_values_test - tests if negative values are sign_extended to 64 bits correctly
*/
static void sign_extend_positive_values_test(void) {

    // Positive values remain unchanged after sign extend

    assert(sign_extend(0x0, 19) == 0);
    assert(sign_extend(0xFF, 13) == 255);
    assert(sign_extend(0x6, 13) == 6);

    printf("sign_extend_positive_values_test: PASSED\n");
}

static void sign_extend_negative_values_test(void) {

    /*
        13-bit value -1 = 0x1FFF

        13-bit value -3 = 0x1FFD

        19-bit value -1 = 0x7FFFF

        26-bit value -4 = 0x3FFFFC
    */

    assert(sign_extend(0x1FFF, 13) == -1);
    assert(sign_extend(0x1FFD, 13) == -3);
    assert(sign_extend(0x7FFFF, 19) == -1);
    assert(sign_extend(0x3FFFFFC, 26) == -4);

    printf("sign_extend_negative_values_test: PASSED\n");
}

/*
    Main function that combines helper test functions to test_bit_utils
*/
int main(void) {
    printf("Testing bit_utils...\n");

    extract_bits_single_bit_test();
    extract_bits_range_test();
    sign_extend_positive_values_test();
    sign_extend_negative_values_test();

    printf("test_bit_utils: ALL TESTS PASSED\n");

    return 0;
}



