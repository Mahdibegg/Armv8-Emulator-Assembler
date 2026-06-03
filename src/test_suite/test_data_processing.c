#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
 
#include "instruction_types/data_processing/data_processing.h"
#include "state.h"
#include "registers/registers.h"
#include "types.h"
#include "test_utils"

// IMM ARITHMETIC TESTS

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
    assert_special_registers_initialised(&state);
 
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
    assert_special_registers_initialised(&state);
 
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
    assert_special_registers_initialised(&state);
 
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
    assert_special_registers_initialised(&state);
 
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