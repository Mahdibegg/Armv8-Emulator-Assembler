#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "pipeline.h"
#include "memory/memory.h"
#include "registers/registers.h"
#include "instruction_types/branch/branch.h"
#include "instruction_types/data_processing/data_processing.h"
#include "instruction_types/load_store/load_store.h"

// no storing variables, all pointers makes it efficient fetch
word_t fetch_instr(const machine_state_t *state) {
    return read_word(&state->memory, read_pc(&state->special_registers));
}

// TODO - WHEN FUNCTION IS COMPLETED DELETE THIS ------------
// decodes instruction
decoded_instr_t decode_instr_type(word_t instruction) {

    // fill in struct to return, passed to exec_result
    decoded_instr_t decoded_instruction = {
        .instr = instruction,
        .type = INSTR_UNKNOWN
    };

    // halting instruction
    if (instruction == 0x8a000000) {
        decoded_instruction.type = INSTR_HALT;
    }

    // --------- CONTINUE HERE, DELETE WHEN ADDED -----------

    return decoded_instruction;
}

// returns the next state in the pipeline
// pattern matches instruction type and passes down the instruction for exact instruction execution
exec_result_t execute_instr(machine_state_t *state, const decoded_instr_t decoded_instr_type) {

    switch (decoded_instr_type.type) {

        case INSTR_HALT:
            return EXEC_HALT;

        case INSTR_DP_IMM:
        case INSTR_DP_REG:
            return execute_data_processing(state, decoded_instr_type);

        case INSTR_LOAD_STORE:
            return execute_load_store(state, decoded_instr_type);

        case INSTR_BRANCH:
            return execute_branch(state, decoded_instr_type);

        default:
            fprintf(stderr, "Unknown instruction: %" PRIu32 "\n", decoded_instr_type.instr);
            exit(EXIT_FAILURE);
    }
}