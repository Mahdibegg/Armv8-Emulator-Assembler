#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "../branch/branch.h"
#include "../state.h"
#include "../registers/registers.h"
#include "../types.h"

static void unconditional_branch_forward_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    write_pc(&state.special_registers.pc, 100);

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
    assert(read_pc(&state.special_registers.pc) == 104);

    printf("Unconditional branch forward: PASSED\n");
}

int main(void) {
    printf("Running branch tests...\n\n");

    test_unconditional_branch_forward();

    printf("\nAll branch tests passed!\n");

    return 0;
}