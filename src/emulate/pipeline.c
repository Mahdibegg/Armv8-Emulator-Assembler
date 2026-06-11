#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "emulate/pipeline.h"
#include "emulate/memory.h"
#include "emulate/registers.h"
#include "emulate/branch.h"
#include "emulate/data_processing.h"
#include "emulate/load_store.h"
#include "shared/bit.h"

#define HALT_INSTR 0x8a000000

#define OP0_HIGH 28
#define OP0_LOW  25

// 100x
#define OP0_MASK_DP_IMM     0xEu
#define OP0_VALUE_DP_IMM    0x8u   

// x101
#define OP0_MASK_DP_REG     0x7u
#define OP0_VALUE_DP_REG    0x5u   

// x1x0
#define OP0_MASK_LOAD_STORE  0x5u
#define OP0_VALUE_LOAD_STORE 0x4u   

// 101x
#define OP0_MASK_BRANCH      0xEu
#define OP0_VALUE_BRANCH     0xAu   

// no storing variables, all pointers makes it efficient fetch
word_t fetch_instr(const machine_state_t *state) {

    // using a memory function to read the current machine state memory with the address inside the PC
    return read_word(&state->memory, read_pc(&state->special_registers));
}

// decodes instruction type includes: INSTR_HALT, INSTR_DP_IMM, INSTR_DP_REG, INSTR_LOAD_STORE, INSTR_BRANCH 
decoded_instr_t decode_instr_type(word_t instruction) {

    // fill in struct to return, passed to exec_result
    decoded_instr_t decoded_instruction = {
        .instr = instruction,
        .type = INSTR_UNKNOWN
    };

    if (instruction == HALT_INSTR) {
        decoded_instruction.type = INSTR_HALT;
        return decoded_instruction;
    }

    word_t op0 = extract_bits(instruction, OP0_LOW, OP0_HIGH);

    // using the mask we are able to ignore the "dont care" bits
    if ((op0 & OP0_MASK_DP_IMM) == OP0_VALUE_DP_IMM) {

        decoded_instruction.type = INSTR_DP_IMM;
    } else if ((op0 & OP0_MASK_DP_REG) == OP0_VALUE_DP_REG) {

        decoded_instruction.type = INSTR_DP_REG;
    } else if ((op0 & OP0_MASK_LOAD_STORE) == OP0_VALUE_LOAD_STORE) {

        decoded_instruction.type = INSTR_LOAD_STORE;
    } else if ((op0 & OP0_MASK_BRANCH) == OP0_VALUE_BRANCH) {

        decoded_instruction.type = INSTR_BRANCH;
    }

    return decoded_instruction;
}

// returns the next state in the pipeline
// pattern matches instruction type and passes down the instruction for exact instruction execution
exec_result_t execute_instr(machine_state_t *state, const decoded_instr_t decoded_instr_type) {

    // checking each case of instructions available to emulate respectively
    switch (decoded_instr_type.type) {

        // exec_halt returned so that in the actual pipeline loop the state will be updated to halt
        case INSTR_HALT:
            return EXEC_HALT;

        // both data processes will have their respective sub executions case handled in a single execution function
        case INSTR_DP_IMM:
        case INSTR_DP_REG:

            return execute_data_processing(state, decoded_instr_type);
        case INSTR_LOAD_STORE:

            return execute_load_store(state, decoded_instr_type);
        case INSTR_BRANCH:


            // the state should be updated, more specifically the PC will be updated with new address
            // the branching will return exec_branch enum which can just be passed over
            return execute_branch(state, decoded_instr_type);
        default:

            // error handling on an unknown decoded instruction type 
            // stops the program before wrongly executing this unkown instruction 
            fprintf(stderr, "Unknown instruction: 0x%" PRIx32 "\n", decoded_instr_type.instr);
            exit(EXIT_FAILURE);
    }
}

// FDE pipeline while loop wrapper
void run_pipeline(machine_state_t *state) {

    state->halted = false;

    while (!state->halted) {

        // FETCH
        word_t instr = fetch_instr(state);

        // DECODE
        decoded_instr_t decoded = decode_instr_type(instr);

        // EXECUTE
        exec_result_t result = execute_instr(state, decoded);

        // Handle result
        switch (result) {

            // Halt loop
            case EXEC_HALT:

                // will stop the pipeline
                state->halted = true;
                break;
            case EXEC_NEXT:

                // move to next instruction (4 bytes ahead)
                // could move this to data_processing and load_store but would be repeated logic
                write_pc(&state->special_registers, read_pc(&state->special_registers) + 4);
                break;
            case EXEC_BRANCH:

                // modify PC in execute_branch()
                // unique logic so better to handle in branch
                break;
        }
    }
}