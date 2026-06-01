<<<<<<< HEAD
#include <stdio.h>
#include <stdlib.h>

#include "data_processing.h"

imm_instr_fields decode_imm_instr(decoded_instr_t instr) {
    void;
}

reg_instr_fields decode_reg_instr(decoded_instr_t instr) {
    void;
}
=======
#include "data_processing.h"
>>>>>>> part1_emulate_setup

exec_result_t execute_data_processing(machine_state_t *state, decoded_instr_t instr) {

    // distinguish between immediate and register instruction
    if (instr.type == INSTR_DP_IMM) {
        decode_imm_instr(instr);
    } else if (instr.type == INSTR_DP_REG) {
        decode_reg_instr(instr);
    } else {

        // code should ideally be unreachable, just for safety

        fprintf(stderr, "Invalid operation: non-immediate/register data process not supported");
        exit(EXIT_FAILURE);
    }

    // no branching, continue to next instruction in pipeline
    return EXEC_NEXT;
}