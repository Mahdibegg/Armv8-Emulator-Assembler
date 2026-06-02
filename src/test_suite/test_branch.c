#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "instruction_types/branch/branch.h"
#include "state.h"
#include "registers/registers.h"
#include "types.h"

/*
 x - Helper functions to test state of general and special purpose reigsters
*/
static void assert_general_registers_are_zero(const machine_state_t *state) {

    for (unsigned i = 0; i < REG_NUM; i++) {
     assert(read_x_register(&state->general_registers, i) == 0);  
    }
}

static void assert_special_registers_initialised_except_pc(const machine_state_t *state, dword_t expected_pc) {

    assert(read_pc(&state->special_registers) == expected_pc);

    assert(state->special_registers.psr.c_flag == false);
    assert(state->special_registers.psr.n_flag == false);
    assert(state->special_registers.psr.v_flag == false);
    assert(state->special_registers.psr.z_flag == true);
}

static void assert_only_x_register_changed(const machine_state_t *state, unsigned changed_index, dword_t expected_value) {
    for (unsigned i = 0; i < REG_NUM; i++) {
        if (i == changed_index) {
            assert(read_x_register(&state->general_registers, i) == expected_value);
        } else {
            assert(read_x_register(&state->general_registers, i) == 0);
        }
    }
}


static void unconditional_branch_forward_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    write_pc(&state.special_registers, 100);

    /*
       Unconditional branch encoding:
       bits 31-26 = 000101 = 0x5
       simm26 = 1

       Actual offset = simm26 << 2 = 1 * 4 = 4

       So PC should become 100 + 4 = 104.
    */
    decoded_instr_t instr;
    instr.type = INSTR_BRANCH;
    instr.instr = (0x5 << 26) | 1;

    exec_result_t result = execute_branch(&state, instr);

    assert(result == EXEC_BRANCH);
    assert(read_pc(&state.special_registers) == 104);

    // Testing that general and special registers remain unchanged after their initialisations
    assert_general_registers_are_zero(&state);
    assert_special_registers_initialised_except_pc(&state, 104);

    printf("Unconditional branch forward: PASSED\n");
}

static void unconditional_branch_backward_test(void) {
    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    write_pc(&state.special_registers, 100);

    /*
       simm26 = -1

       In 26-bit two's complement, -1 is all 26 bits set to 1:
       0x03FFFFFF

       Actual offset = -1 << 2 = -4

       PC should become 100 - 4 = 96.
    */
    decoded_instr_t instr;
    instr.type = INSTR_BRANCH;
    instr.instr = (0x5 << 26) | 0x03FFFFFF;

    exec_result_t result = execute_branch(&state, instr);

    assert(result == EXEC_BRANCH);
    assert(read_pc(&state.special_registers) == 96);

    // Testing that general and special registers remain unchanged after their initialisations
    assert_general_registers_are_zero(&state);
    assert_special_registers_initialised_except_pc(&state, 96);

    printf("Unconditional branch backward: PASSED\n");
}

static void register_branch_test(void) {
    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    write_pc(&state.special_registers, 100);
    write_x_register(&state.general_registers, 3, 500);

    /*
       Register branch pattern:
       1101011000011111000000 Xn 00000

       Fixed bits 31-10 = 0x3587C0
       Xn = 3
    */
    decoded_instr_t instr;
    instr.type = INSTR_BRANCH;
    instr.instr = (0x3587C0 << 10) | (3 << 5);

    exec_result_t result = execute_branch(&state, instr);

    assert(result == EXEC_BRANCH);
    assert(read_pc(&state.special_registers) == 500);

    // Testing that only register 3 has been changed and special registesr are all initialised
    assert_special_registers_initialised_except_pc(&state, 500);
    assert_only_x_register_changed(&state, 3, 500);
    printf("Register branch: PASSED\n");
}

static void conditional_branch_taken_eq_test(void) {
    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    write_pc(&state.special_registers, 100);

    /*
       Conditional branch format:
       01010100 simm19 0 cond

       0x54 = 01010100
       simm19 = 1
       cond = 0x0 for EQ

       Actual offset = 1 << 2 = 4

       Since Z flag is true, PC should become 104.
    */
    decoded_instr_t instr;
    instr.type = INSTR_BRANCH;
    instr.instr = (0x54 << 24) | (1 << 5) | 0x0;

    exec_result_t result = execute_branch(&state, instr);

    assert(result == EXEC_BRANCH);
    assert(read_pc(&state.special_registers) == 104);

    // Testing general registers and special registers
    assert_general_registers_are_zero(&state);
    assert_special_registers_initialised_except_pc(&state, 104);

    printf("Conditional branch taken EQ: PASSED\n");
}

static void conditional_branch_not_taken_ne_test(void) {
    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    write_pc(&state.special_registers, 100);

    /*
       cond = 0x1 for NE

       NE requires Z == 0.
       But z_flag is true, so PC should stay 100.
    */
    decoded_instr_t instr;
    instr.type = INSTR_BRANCH;
    instr.instr = (0x54 << 24) | (1 << 5) | 0x1;

    exec_result_t result = execute_branch(&state, instr);

    assert(result == EXEC_BRANCH);
    assert(read_pc(&state.special_registers) == 100);

    printf("Conditional branch not taken NE: PASSED\n");
}

int main(void) {
    printf("Running branch tests...\n\n");

    unconditional_branch_forward_test();
    unconditional_branch_backward_test();
    register_branch_test();
    conditional_branch_taken_eq_test();
    conditional_branch_not_taken_ne_test();

    printf("\nAll branch tests passed!\n");

    return 0;
}