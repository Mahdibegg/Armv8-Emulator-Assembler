#ifndef LOAD_STORE_H
#define LOAD_STORE_H

#include "emulate/state.h"
#include "emulate/decode_struct.h"
#include "shared/types.h"
#include "shared/instruction_fields.h"

// takes bits and executes the correct load/store type instruction
exec_result_t execute_load_store(machine_state_t *state, decoded_instr_t instr);

#endif