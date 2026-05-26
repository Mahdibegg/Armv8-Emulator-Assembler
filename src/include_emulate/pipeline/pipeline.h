#ifndef PIPELINE_H
#define PIPELINE_H

#include "types.h"
#include "state.h"
#include "decode_struct.h"

// fetch instruction only reads (typealias for memory reading)
word_t fetch_instr(const machine_state_t *state);

// result of fetch_instr passed for decoding
decoded_instr_t decode_instr(const word_t instr);

// state will be updated, hence it's a pointer
exec_result_t execute_instr(machine_state_t *state, const decoded_instr_t instruction);

#endif