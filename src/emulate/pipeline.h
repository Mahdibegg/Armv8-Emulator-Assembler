#ifndef PIPELINE_H
#define PIPELINE_H

#include "shared/types.h"
#include "emulate/state.h"
#include "emulate/decode_struct.h"

/*
 * Fetch instruction only reads (typealias for memory reading)
 * reads the word at the address held in the PC and returns it
 */
word_t fetch_instr(const machine_state_t *state);

/*
 * Result of fetch_instr passed for decoding
 * classifies the raw instruction into a decoded_instr_t
 */
decoded_instr_t decode_instr(const word_t instr);

/*
 * State will be updated, hence it's a pointer
 * executes the decoded instruction and returns the pipeline result
 */
exec_result_t execute_instr(machine_state_t *state, const decoded_instr_t instruction);

/*
 * Wrapper for FDE loop
 * runs the fetch-decode-execute cycle until the machine halts
 */
void run_pipeline(machine_state_t *state);

#endif
