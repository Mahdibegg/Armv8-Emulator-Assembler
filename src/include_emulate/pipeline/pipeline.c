#include "pipeline.h"
#include "memory/memory.h"
#include "registers/registers.h"

// no storing variables, all pointers makes it efficient fetch
word_t fetch_instr(const machine_state_t *state) {
    return read_word(&state->memory, read_pc(&state->special_registers));
}

// TO BE COMPLETED
//decoded_instr_t decode_instr(const word_t instr) {}

// TO BE COMPLETED
// exec_result_t execute_instr(machine_state_t *state, const decoded_instr_t instruction) {}