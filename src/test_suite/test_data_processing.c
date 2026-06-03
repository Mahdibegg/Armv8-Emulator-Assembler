#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
 
#include "instruction_types/data_processing/data_processing.h"
#include "state.h"
#include "registers/registers.h"
#include "types.h"
#include "test_utils/test_utils.h"

#define BLUE "\033[34m"
#define WHITE "\033[0m"

// IMM ARITHMETIC TESTS 1 - 1.9


// TEST 1.1: imm_arithmetic_add_32 (32-bit ADD)
// Testing that 32-bit ADD correctly adds an unshifted immediate value
static void imm_arithmetic_add_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 10);
 
    /*

       Immediate arithmetic ADD (32-bit):
       sf = 0, opc = 00 (ADD), opi = 010, sh = 0
       imm12 = 5, rn = 1, rd = 0
       Op2 = 5 (no shift)
       Result = 10 + 5 = 15

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x1 << 28) | (0x2 << 23) | (0 << 22) | (5 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 15);
 
    // Testing that special registers remain unchanged after add without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("IMM arithmetic ADD 32-bit: PASSED\n");
}
 
// TEST 1.2: imm_arithmetic_add_32_shifted (32-bit ADD shifted)
// Testing that 32-bit ADD correctly applies sh = 1 before adding the immediate value
static void imm_arithmetic_add_32_shifted_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 2, 0);
 
    /*

       Immediate arithmetic ADD (32-bit) with sh = 1:
       sf = 0, opc = 00 (ADD), opi = 010, sh = 1
       imm12 = 1, rn = 2, rd = 3
       Op2 = 1 << 12 = 4096
       Result = 0 + 4096 = 4096

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x1 << 28) | (0x2 << 23) | (1 << 22) | (1 << 10) | (2 << 5) | 3;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 3) == 4096);
 
    // Testing that special registers remain unchanged after add without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("IMM arithmetic ADD 32-bit shifted: PASSED\n");
}
 
// TEST 1.3: imm_arithmetic_adds_32_sets_flags (32-bit ADDS carry)
// Testing that 32-bit ADDS correctly sets carry and zero flags after unsigned overflow
static void imm_arithmetic_adds_32_sets_flags_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 0xFFFFFFFF);
 
    /*

       Immediate arithmetic ADDS (32-bit) with carry:
       sf = 0, opc = 01 (ADDS), opi = 010, sh = 0
       imm12 = 1, rn = 1, rd = 0
       Result = 0xFFFFFFFF + 1 = 0 (carry out, z flag set, c flag set)

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x0 << 31) | (0x1 << 29) | (0x1 << 28) | (0x2 << 23) | (0 << 22) | (1 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0);
 
    // Testing that carry and zero flags are set after unsigned overflow
    assert(state.special_registers.psr.z_flag == true);
    assert(state.special_registers.psr.c_flag == true);
    assert(state.special_registers.psr.n_flag == false);
    assert(state.special_registers.psr.v_flag == false);
 
    printf("IMM arithmetic ADDS 32-bit sets flags (carry): PASSED\n");
}
 
// TEST 1.4: imm_arithmetic_adds_32_signed_overflow (32-bit ADDS signed overflow)
// Testing that 32-bit ADDS correctly sets negative and overflow flags after signed overflow
static void imm_arithmetic_adds_32_signed_overflow_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 0x7FFFFFFF);
 
    /*

       Immediate arithmetic ADDS (32-bit) with signed overflow:
       sf = 0, opc = 01 (ADDS), opi = 010, sh = 0
       imm12 = 1, rn = 1, rd = 0
       Result = 0x7FFFFFFF + 1 = 0x80000000 (signed overflow, n flag set, v flag set)

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x0 << 31) | (0x1 << 29) | (0x1 << 28) | (0x2 << 23) | (0 << 22) | (1 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0x80000000);
 
    // Testing that n and v flags are set after signed overflow
    assert(state.special_registers.psr.n_flag == true);
    assert(state.special_registers.psr.v_flag == true);
    assert(state.special_registers.psr.z_flag == false);
    assert(state.special_registers.psr.c_flag == false);
 
    printf("IMM arithmetic ADDS 32-bit sets flags (signed overflow): PASSED\n");
}
 
// TEST 1.5: imm_arithmetic_sub_32 (32-bit SUB)
// Testing that 32-bit SUB correctly subtracts an unshifted immediate value
static void imm_arithmetic_sub_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 20);
 
    /*

       Immediate arithmetic SUB (32-bit):
       sf = 0, opc = 10 (SUB), opi = 010, sh = 0
       imm12 = 5, rn = 1, rd = 0
       Result = 20 - 5 = 15

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x0 << 31) | (0x2 << 29) | (0x1 << 28) | (0x2 << 23) | (0 << 22) | (5 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 15);
 
    // Testing that special registers remain unchanged after sub without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("IMM arithmetic SUB 32-bit: PASSED\n");
}
 
// TEST 1.6: imm_arithmetic_subs_32_sets_flags (32-bit SUBS borrow)
// Testing that 32-bit SUBS correctly sets negative flag and clears carry flag after borrow
static void imm_arithmetic_subs_32_sets_flags_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 5);
 
    /*

       Immediate arithmetic SUBS (32-bit) with borrow:
       sf = 0, opc = 11 (SUBS), opi = 010, sh = 0
       imm12 = 10, rn = 1, rd = 0
       Result = 5 - 10 = underflow (borrow occurs, c flag = 0, n flag set)

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x0 << 31) | (0x3 << 29) | (0x1 << 28) | (0x2 << 23) | (0 << 22) | (10 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
 
    // Testing that n flag is set and c flag is 0 after borrow
    assert(state.special_registers.psr.n_flag == true);
    assert(state.special_registers.psr.c_flag == false);
    assert(state.special_registers.psr.z_flag == false);
 
    printf("IMM arithmetic SUBS 32-bit sets flags (borrow): PASSED\n");
}
 
// TEST 1.7: imm_arithmetic_subs_32_zero_result (32-bit SUBS zero result)
// Testing that 32-bit SUBS correctly sets zero and carry flags when subtraction gives zero
static void imm_arithmetic_subs_32_zero_result_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 10);
 
    /*

       Immediate arithmetic SUBS (32-bit) resulting in zero:
       sf = 0, opc = 11 (SUBS), opi = 010, sh = 0
       imm12 = 10, rn = 1, rd = 0
       Result = 10 - 10 = 0 (z flag set, c flag set - no borrow)

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x0 << 31) | (0x3 << 29) | (0x1 << 28) | (0x2 << 23) | (0 << 22) | (10 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0);
 
    // Testing that z flag is set and c flag is 1 (no borrow) after equal subtraction
    assert(state.special_registers.psr.z_flag == true);
    assert(state.special_registers.psr.c_flag == true);
    assert(state.special_registers.psr.n_flag == false);
    assert(state.special_registers.psr.v_flag == false);
 
    printf("IMM arithmetic SUBS 32-bit zero result: PASSED\n");
}
 
// TEST 1.8: imm_arithmetic_add_64 (64-bit ADD)
// Testing that 64-bit ADD correctly adds an unshifted immediate value
static void imm_arithmetic_add_64_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_x_register(&state.general_registers, 1, 0x100000000);
 
    /*

       Immediate arithmetic ADD (64-bit):
       sf = 1, opc = 00 (ADD), opi = 010, sh = 0
       imm12 = 1, rn = 1, rd = 0
       Result = 0x100000000 + 1 = 0x100000001

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x1 << 31) | (0x0 << 29) | (0x1 << 28) | (0x2 << 23) | (0 << 22) | (1 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 0) == 0x100000001);
 
    // Testing that special registers remain unchanged after 64-bit add without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("IMM arithmetic ADD 64-bit: PASSED\n");
}
 
// TEST 1.9: imm_arithmetic_adds_64_carry (64-bit ADDS carry)
// Testing that 64-bit ADDS correctly sets carry and zero flags after unsigned overflow
static void imm_arithmetic_adds_64_carry_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_x_register(&state.general_registers, 1, 0xFFFFFFFFFFFFFFFF);
 
    /*

       Immediate arithmetic ADDS (64-bit) with carry:
       sf = 1, opc = 01 (ADDS), opi = 010, sh = 0
       imm12 = 1, rn = 1, rd = 0
       Result = 0xFFFFFFFFFFFFFFFF + 1 = 0 (carry, z flag set, c flag set)

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x1 << 31) | (0x1 << 29) | (0x1 << 28) | (0x2 << 23) | (0 << 22) | (1 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 0) == 0);
 
    // Testing that carry and zero flags are set after 64-bit unsigned overflow
    assert(state.special_registers.psr.z_flag == true);
    assert(state.special_registers.psr.c_flag == true);
    assert(state.special_registers.psr.n_flag == false);
    assert(state.special_registers.psr.v_flag == false);
 
    printf("IMM arithmetic ADDS 64-bit carry: PASSED\n");
}


// IMM WIDE MOVE TESTS 2 - 2.6


// TEST 2.1: imm_wide_move_movz_32
// Testing that 32-bit MOVZ correctly writes an unshifted 16-bit immediate value
static void imm_wide_move_movz_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    /*

       Immediate wide move MOVZ (32-bit):
       sf = 0, opc = 10 (MOVZ), opi = 101, hw = 0
       imm16 = 0xABCD, rd = 0
       Op = 0xABCD << 0 = 0xABCD
       Result = 0xABCD

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x0 << 31) | (0x2 << 29) | (0x1 << 28) | (0x5 << 23) | (0 << 21) | (0xABCD << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0xABCD);
 
    // Testing that special registers and other general registers remain unchanged
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("IMM wide move MOVZ 32-bit: PASSED\n");
}
 
// TEST 2.2: imm_wide_move_movz_32_shifted
// Testing that 32-bit MOVZ correctly applies hw = 1 before writing the immediate value
static void imm_wide_move_movz_32_shifted_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    /*

       Immediate wide move MOVZ (32-bit) with hw = 1:
       sf = 0, opc = 10 (MOVZ), opi = 101, hw = 1
       imm16 = 0x1, rd = 0
       Op = 0x1 << 16 = 0x10000
       Result = 0x10000

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x0 << 31) | (0x2 << 29) | (0x1 << 28) | (0x5 << 23) | (1 << 21) | (0x1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0x10000);
 
    // Testing that special registers remain unchanged after movz
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("IMM wide move MOVZ 32-bit shifted: PASSED\n");
}
 
// TEST 2.3: imm_wide_move_movn_32
// Testing that 32-bit MOVN correctly writes the bitwise negation of the immediate value
static void imm_wide_move_movn_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    /*

       Immediate wide move MOVN (32-bit):
       sf = 0, opc = 00 (MOVN), opi = 101, hw = 0
       imm16 = 0, rd = 0
       Op = 0 << 0 = 0
       Result = ~0 truncated to 32 bits = 0xFFFFFFFF

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x1 << 28) | (0x5 << 23) | (0 << 21) | (0x0 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0xFFFFFFFF);
 
    // Testing that special registers remain unchanged after movn
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("IMM wide move MOVN 32-bit: PASSED\n");
}
 
// TEST 2.4: imm_wide_move_movk_32
// Testing that 32-bit MOVK correctly keeps existing bits while replacing bits 15 - 0
static void imm_wide_move_movk_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 0, 0xFFFFFFFF);
 
    /*

       Immediate wide move MOVK (32-bit):
       sf = 0, opc = 11 (MOVK), opi = 101, hw = 0
       imm16 = 0x00FF, rd = 0
       Rd was 0xFFFFFFFF, inserting 0x00FF at bits 15:0
       Result = 0xFFFF00FF

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x0 << 31) | (0x3 << 29) | (0x1 << 28) | (0x5 << 23) | (0 << 21) | (0x00FF << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0xFFFF00FF);
 
    // Testing that special registers remain unchanged after movk
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("IMM wide move MOVK 32-bit: PASSED\n");
}
 
// TEST 2.5: imm_wide_move_movz_64
// Testing that 64-bit MOVZ correctly writes an unshifted 16-bit immediate value
static void imm_wide_move_movz_64_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    /*

       Immediate wide move MOVZ (64-bit):
       sf = 1, opc = 10 (MOVZ), opi = 101, hw = 0
       imm16 = 0x1234, rd = 5
       Result = 0x1234

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x1 << 31) | (0x2 << 29) | (0x1 << 28) | (0x5 << 23) | (0 << 21) | (0x1234 << 5) | 5;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 5) == 0x1234);
 
    // Testing that special registers and other general registers remain unchanged
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
    assert_only_x_register_changed(&state, 5, 0x1234);
 
    printf("IMM wide move MOVZ 64-bit: PASSED\n");
}
 
// TEST 2.6: imm_wide_move_movk_64_upper_bits
// Testing that 64-bit MOVK correctly inserts the immediate value into the upper 16 bits
static void imm_wide_move_movk_64_upper_bits_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_x_register(&state.general_registers, 0, 0x0000000000000000);
 
    /*

       Immediate wide move MOVK (64-bit) inserting into upper bits:
       sf = 1, opc = 11 (MOVK), opi = 101, hw = 3
       imm16 = 0x00FF, rd = 0
       Inserting 0x00FF at bits 63:48
       Result = 0x00FF000000000000

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_IMM;
    instr.instr = (0x1 << 31) | (0x3 << 29) | (0x1 << 28) | (0x5 << 23) | (3 << 21) | (0x00FF << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 0) == 0x00FF000000000000);
 
    // Testing that special registers remain unchanged after movk
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("IMM wide move MOVK 64-bit upper bits: PASSED\n");
}


// REG ARITHMETIC TESTS 3 - 3.5
 

// TEST 3.1: reg_arithmetic_add_lsl_32
// Testing that 32-bit ADD correctly applies LSL to the register operand before adding
static void reg_arithmetic_add_lsl_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 10);
    write_w_register(&state.general_registers, 2, 1);
 
    /*

       Register arithmetic ADD (32-bit) with LSL:
       sf = 0, opc = 00 (ADD), M = 0, opr = 1000, shift = 00 (LSL)
       operand (shift_amount) = 2, rn = 1, rm = 2, rd = 0
       Op2 = 1 << 2 = 4
       Result = 10 + 4 = 14

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x1 << 24) | (0x0 << 22) | (2 << 16) | (2 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 14);
 
    // Testing that special registers remain unchanged after add without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("REG arithmetic ADD 32-bit LSL: PASSED\n");
}
 
// TEST 3.2: reg_arithmetic_add_lsr_32
// Testing that 32-bit ADD correctly applies LSR to the register operand before adding
static void reg_arithmetic_add_lsr_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 10);
    write_w_register(&state.general_registers, 2, 8);
 
    /*

       Register arithmetic ADD (32-bit) with LSR:
       sf = 0, opc = 00 (ADD), shift = 01 (LSR)
       shift_amount = 1, rn = 1, rm = 2, rd = 0
       Op2 = 8 >> 1 = 4 (vacated bits filled with 0)
       Result = 10 + 4 = 14

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x1 << 24) | (0x1 << 22) | (2 << 16) | (1 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 14);
 
    // Testing that special registers remain unchanged after add without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("REG arithmetic ADD 32-bit LSR: PASSED\n");
}
 
// TEST 3.3: reg_arithmetic_add_asr_32
// Testing that 32-bit ADD correctly applies ASR while preserving the sign bit before adding
static void reg_arithmetic_add_asr_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 0);
    write_w_register(&state.general_registers, 2, 0x80000000);
 
    /*

       Register arithmetic ADD (32-bit) with ASR:
       sf = 0, opc = 00 (ADD), shift = 10 (ASR)
       shift_amount = 1, rn = 1, rm = 2, rd = 0
       Op2 = 0x80000000 >> 1 = 0xC0000000 (sign bit preserved)
       Result = 0 + 0xC0000000 = 0xC0000000

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x1 << 24) | (0x2 << 22) | (2 << 16) | (1 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0xC0000000);
 
    // Testing that special registers remain unchanged after add without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("REG arithmetic ADD 32-bit ASR (sign preserved): PASSED\n");
}
 
// TEST 3.4: reg_arithmetic_add_ror_32
// Testing that 32-bit ADD correctly applies ROR to the register operand before adding
static void reg_arithmetic_add_ror_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 0);
    write_w_register(&state.general_registers, 2, 0x00000001);
 
    /*

       Register arithmetic ADD (32-bit) with ROR:
       sf = 0, opc = 00 (ADD), shift = 11 (ROR)
       shift_amount = 1, rn = 1, rm = 2, rd = 0
       Op2 = rotate_right(0x00000001, 1) = 0x80000000
       Result = 0 + 0x80000000 = 0x80000000

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x1 << 24) | (0x3 << 22) | (2 << 16) | (1 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0x80000000);
 
    // Testing that special registers remain unchanged after add without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("REG arithmetic ADD 32-bit ROR: PASSED\n");
}
 
// TEST 3.5: reg_arithmetic_sub_64
// Testing that 64-bit SUB correctly subtracts the shifted register operand
static void reg_arithmetic_sub_64_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_x_register(&state.general_registers, 1, 0x200000000);
    write_x_register(&state.general_registers, 2, 0x100000000);
 
    /*

       Register arithmetic SUB (64-bit) with LSL shift amount 0:
       sf = 1, opc = 10 (SUB), shift = 00 (LSL), shift_amount = 0
       rn = 1, rm = 2, rd = 0
       Op2 = 0x100000000 << 0 = 0x100000000
       Result = 0x200000000 - 0x100000000 = 0x100000000

    */

    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x1 << 31) | (0x2 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x1 << 24) | (0x0 << 22) | (2 << 16) | (0 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 0) == 0x100000000);
 
    // Testing that special registers remain unchanged after sub without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("REG arithmetic SUB 64-bit: PASSED\n");
}


// REG LOGIC TESTS 4 - 4.7


// TEST 4.1: reg_logic_and_32
// Testing that 32-bit AND correctly performs a bitwise AND between two register operands
static void reg_logic_and_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 0xFF00FF00);
    write_w_register(&state.general_registers, 2, 0xFFFF0000);
 
    /*
       Register logic AND (32-bit):
       sf = 0, opc = 00, N = 0, M = 0, opr = 0000
       rn = 1, rm = 2, rd = 0
       Result = 0xFF00FF00 & 0xFFFF0000 = 0xFF000000
    */
    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x0 << 24) | (0x0 << 22) | (2 << 16) | (0 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0xFF000000);
 
    // Testing that special registers remain unchanged after and without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("REG logic AND 32-bit: PASSED\n");
}
 
// TEST 4.2: reg_logic_orr_32
// Testing that 32-bit ORR correctly performs a bitwise OR between two register operands
static void reg_logic_orr_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 0xFF000000);
    write_w_register(&state.general_registers, 2, 0x00FF0000);
 
    /*
       Register logic ORR (32-bit):
       sf = 0, opc = 01, N = 0, M = 0, opr = 0000
       rn = 1, rm = 2, rd = 0
       Result = 0xFF000000 | 0x00FF0000 = 0xFFFF0000
    */
    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x1 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x0 << 24) | (0x0 << 22) | (2 << 16) | (0 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0xFFFF0000);
 
    // Testing that special registers remain unchanged after orr without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("REG logic ORR 32-bit: PASSED\n");
}
 
// TEST 4.3: reg_logic_eor_32
// Testing that 32-bit EOR correctly performs a bitwise exclusive OR between two register operands
static void reg_logic_eor_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 0xFFFFFFFF);
    write_w_register(&state.general_registers, 2, 0xFFFFFFFF);
 
    /*
       Register logic EOR (32-bit):
       sf = 0, opc = 10, N = 0, M = 0, opr = 0000
       rn = 1, rm = 2, rd = 0
       Result = 0xFFFFFFFF ^ 0xFFFFFFFF = 0x0
    */
    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x2 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x0 << 24) | (0x0 << 22) | (2 << 16) | (0 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0x0);
 
    // Testing that special registers remain unchanged after eor without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("REG logic EOR 32-bit: PASSED\n");
}
 
// TEST 4.4: reg_logic_bic_32
// Testing that 32-bit BIC correctly performs a bitwise AND with the negated register operand
static void reg_logic_bic_32_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 0xFFFFFFFF);
    write_w_register(&state.general_registers, 2, 0xFF000000);
 
    /*
       Register logic BIC (32-bit):
       sf = 0, opc = 00, N = 1 (BIC), M = 0, opr = 0001
       rn = 1, rm = 2, rd = 0
       Result = 0xFFFFFFFF & ~0xFF000000 = 0x00FFFFFF
    */
    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x0 << 24) | (0x0 << 22) | (2 << 16) | (0 << 10) | (1 << 5) | 0 | (0x1 << 21);
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0x00FFFFFF);
 
    // Testing that special registers remain unchanged after bic without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("REG logic BIC 32-bit: PASSED\n");
}
 
// TEST 4.5: reg_logic_ands_32_sets_flags
// Testing that 32-bit ANDS correctly sets the zero flag when the bitwise result is zero
static void reg_logic_ands_32_sets_flags_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 0x00000000);
    write_w_register(&state.general_registers, 2, 0xFFFFFFFF);
 
    /*
       Register logic ANDS (32-bit) resulting in zero:
       sf = 0, opc = 11, N = 0 (ANDS), M = 0, opr = 0110
       rn = 1, rm = 2, rd = 0
       Result = 0x00000000 & 0xFFFFFFFF = 0x0 (z flag set, c = v = 0)
    */
    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x3 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x0 << 24) | (0x0 << 22) | (2 << 16) | (0 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0x0);
 
    // Testing that z flag is set and c, v flags are 0 after ands with zero result
    assert(state.special_registers.psr.z_flag == true);
    assert(state.special_registers.psr.n_flag == false);
    assert(state.special_registers.psr.c_flag == false);
    assert(state.special_registers.psr.v_flag == false);
 
    printf("REG logic ANDS 32-bit sets flags (zero result): PASSED\n");
}
 
// TEST 4.6: reg_logic_ands_32_negative_result
// Testing that 32-bit ANDS correctly sets the negative flag when the sign bit is set
static void reg_logic_ands_32_negative_result_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_w_register(&state.general_registers, 1, 0x80000000);
    write_w_register(&state.general_registers, 2, 0x80000000);
 
    /*
       Register logic ANDS (32-bit) with negative result:
       sf = 0, opc = 11, N = 0 (ANDS)
       rn = 1, rm = 2, rd = 0
       Result = 0x80000000 & 0x80000000 = 0x80000000 (n flag set, c = v = 0)
    */
    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x3 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x0 << 24) | (0x0 << 22) | (2 << 16) | (0 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 0x80000000);
 
    // Testing that n flag is set and c, v flags are 0 after ands with negative result
    assert(state.special_registers.psr.n_flag == true);
    assert(state.special_registers.psr.z_flag == false);
    assert(state.special_registers.psr.c_flag == false);
    assert(state.special_registers.psr.v_flag == false);
 
    printf("REG logic ANDS 32-bit sets flags (negative result): PASSED\n");
}
 
// TEST 4.7: reg_logic_orn_64
// Testing that 64-bit ORN correctly performs a bitwise OR with the negated register operand
static void reg_logic_orn_64_test(void) {
 
    machine_state_t state;
 
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
 
    write_x_register(&state.general_registers, 1, 0x0000000000000000);
    write_x_register(&state.general_registers, 2, 0xFFFFFFFF00000000);
 
    /*
       Register logic ORN (64-bit):
       sf = 1, opc = 01, N = 1 (ORN)
       rn = 1, rm = 2, rd = 0
       Result = 0x0 | ~0xFFFFFFFF00000000 = 0x00000000FFFFFFFF
    */
    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x1 << 31) | (0x1 << 29) | (0x0 << 28) | (0x1 << 27) | (0x0 << 26) | (0x1 << 25) |
                  (0x0 << 24) | (0x0 << 22) | (0x1 << 21) | (2 << 16) | (0 << 10) | (1 << 5) | 0;
 
    exec_result_t result = execute_data_processing(&state, instr);
 
    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 0) == 0x00000000FFFFFFFF);
 
    // Testing that special registers remain unchanged after orn without flags
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);
 
    printf("REG logic ORN 64-bit: PASSED\n");
}


// REG MULTIPLY TESTS 5 - 5.4

// TEST 5.1: reg_multiply_madd_32
// Testing that 32-bit MADD correctly adds the product of two registers to the accumulator register
static void reg_multiply_madd_32_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    write_w_register(&state.general_registers, 1, 3);
    write_w_register(&state.general_registers, 2, 4);
    write_w_register(&state.general_registers, 3, 10);

    /*
       Register multiply MADD (32-bit):
       sf = 0, opc = 00, M = 1, fixed = 1101 1000, x = 0 (MADD)
       rn = 1, rm = 2, ra = 3, rd = 0
       Result = ra + (rn * rm) = 10 + (3 * 4) = 22
    */
    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x0 << 25) |
                  (0x1 << 24) | (0x0 << 21) | (2 << 16) | (0 << 15) | (3 << 10) | (1 << 5) | 0;

    exec_result_t result = execute_data_processing(&state, instr);

    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 22);

    // Testing that special registers remain unchanged after madd
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);

    printf("REG multiply MADD 32-bit: PASSED\n");
}

// TEST 5.2: reg_multiply_msub_32
// Testing that 32-bit MSUB correctly subtracts the product of two registers from the accumulator register
static void reg_multiply_msub_32_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    write_w_register(&state.general_registers, 1, 3);
    write_w_register(&state.general_registers, 2, 4);
    write_w_register(&state.general_registers, 3, 20);

    /*
       Register multiply MSUB (32-bit):
       sf = 0, opc = 00, M = 1, fixed = 1101 1000, x = 1 (MSUB)
       rn = 1, rm = 2, ra = 3, rd = 0
       Result = ra - (rn * rm) = 20 - (3 * 4) = 8
    */
    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x0 << 25) |
                  (0x1 << 24) | (0x0 << 21) | (2 << 16) | (0x1 << 15) | (3 << 10) | (1 << 5) | 0;

    exec_result_t result = execute_data_processing(&state, instr);

    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 8);

    // Testing that special registers remain unchanged after msub
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);

    printf("REG multiply MSUB 32-bit: PASSED\n");
}

// TEST 5.3: reg_multiply_madd_zero_ra_32
// Testing that 32-bit MADD correctly uses the zero register as the accumulator value
static void reg_multiply_madd_zero_ra_32_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    write_w_register(&state.general_registers, 1, 6);
    write_w_register(&state.general_registers, 2, 7);

    /*
       Register multiply MADD (32-bit) with ra = zero register:
       sf = 0, opc = 00, M = 1, fixed = 1101 1000, x = 0 (MADD)
       rn = 1, rm = 2, ra = 31 (zero register), rd = 0
       Result = 0 + (6 * 7) = 42
    */
    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x0 << 31) | (0x0 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x0 << 25) |
                  (0x1 << 24) | (0x0 << 21) | (2 << 16) | (0x0 << 15) | (31 << 10) | (1 << 5) | 0;

    exec_result_t result = execute_data_processing(&state, instr);

    assert(result == EXEC_NEXT);
    assert(read_w_register(&state.general_registers, 0) == 42);

    // Testing that special registers remain unchanged after madd with zero register
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);

    printf("REG multiply MADD 32-bit (ra = zero register): PASSED\n");
}

// TEST 5.4: reg_multiply_madd_64
// Testing that 64-bit MADD correctly multiplies large 64-bit register values
static void reg_multiply_madd_64_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    write_x_register(&state.general_registers, 1, 0x100000000);
    write_x_register(&state.general_registers, 2, 2);
    write_x_register(&state.general_registers, 3, 0);

    /*
       Register multiply MADD (64-bit):
       sf = 1, opc = 00, M = 1, fixed = 1101 1000, x = 0 (MADD)
       rn = 1, rm = 2, ra = 3, rd = 0
       Result = 0 + (0x100000000 * 2) = 0x200000000
    */
    decoded_instr_t instr;
    instr.type = INSTR_DP_REG;
    instr.instr = (0x1 << 31) | (0x0 << 29) | (0x1 << 28) | (0x1 << 27) | (0x1 << 26) | (0x0 << 25) |
                  (0x1 << 24) | (0x0 << 21) | (2 << 16) | (0x0 << 15) | (3 << 10) | (1 << 5) | 0;

    exec_result_t result = execute_data_processing(&state, instr);

    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 0) == 0x200000000);

    // Testing that special registers remain unchanged after 64-bit madd
    assert_special_registers_initialised_except_pc(&state, (dword_t) 0x0);

    printf("REG multiply MADD 64-bit: PASSED\n");
}

int main(void) {

    printf("Running data processing tests...\n\n");
 
    printf("IMM ARITHMETIC TESTS --->\n");
    imm_arithmetic_add_32_test();
    imm_arithmetic_add_32_shifted_test();
    imm_arithmetic_adds_32_sets_flags_test();
    imm_arithmetic_adds_32_signed_overflow_test();
    imm_arithmetic_sub_32_test();
    imm_arithmetic_subs_32_sets_flags_test();
    imm_arithmetic_subs_32_zero_result_test();
    imm_arithmetic_add_64_test();
    imm_arithmetic_adds_64_carry_test();

    printf("\nIMM WIDE MOVE TESTS --->\n");
    imm_wide_move_movz_32_test();
    imm_wide_move_movz_32_shifted_test();
    imm_wide_move_movn_32_test();
    imm_wide_move_movk_32_test();
    imm_wide_move_movz_64_test();
    imm_wide_move_movk_64_upper_bits_test();

    printf("\nREG ARITHMETIC TESTS --->\n");
    reg_arithmetic_add_lsl_32_test();
    reg_arithmetic_add_lsr_32_test();
    reg_arithmetic_add_asr_32_test();
    reg_arithmetic_add_ror_32_test();
    reg_arithmetic_sub_64_test();
 
    printf("\nREG Logic --->\n");
    reg_logic_and_32_test();
    reg_logic_orr_32_test();
    reg_logic_eor_32_test();
    reg_logic_bic_32_test();
    reg_logic_ands_32_sets_flags_test();
    reg_logic_ands_32_negative_result_test();
    reg_logic_orn_64_test();

    printf("\nREG Multiply --->\n");
    reg_multiply_madd_32_test();
    reg_multiply_msub_32_test();
    reg_multiply_madd_zero_ra_32_test();
    reg_multiply_madd_64_test();

    printf(BLUE "\nAll data processing tests PASSED\n" WHITE);
 
    return 0;
}