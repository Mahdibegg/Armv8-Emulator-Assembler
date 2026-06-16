#ifndef LOAD_STORE_H
#define LOAD_STORE_H

#include "emulate/state.h"
#include "shared/decode.h"
#include "shared/types.h"
#include "shared/instruction_fields.h"

/*
 * Takes bits and executes the correct load/store type instruction
 * decodes the instruction then performs the load or store on the machine state
 */
exec_result_t execute_load_store(machine_state_t *state, decoded_instr_t instr);

#endif