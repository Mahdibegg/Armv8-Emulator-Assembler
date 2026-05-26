#ifndef LOAD_STORE_H
#define LOAD_STORE_H

#include "state.h"
#include "pipeline/decode_struct.h"

exec_result_t execute_load_store(machine_state_t *state, decoded_instr_t instr);

#endif