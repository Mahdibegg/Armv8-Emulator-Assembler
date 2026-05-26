#ifndef BRANCH_H
#define BRANCH_H

#include "state.h"
#include "pipeline/decode_struct.h"

exec_result_t execute_branch(machine_state_t *state, decoded_instr_t instr);

#endif