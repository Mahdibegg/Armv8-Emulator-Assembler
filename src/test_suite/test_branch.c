#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "instruction_types/branch/branch.h"
#include "state.h"
#include "registers/registers.h"
#include "types.h"

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

    printf("Unconditional branch backward: PASSED\n");
}