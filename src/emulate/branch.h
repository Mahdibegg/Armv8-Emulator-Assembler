#ifndef BRANCH_H
#define BRANCH_H

#include "emulate/state.h"
#include "shared/decode.h"
#include "shared/types.h"
#include "shared/instruction_fields.h"
#include <stdint.h>

/*
 * Used in pipeline to update the PC value appropriately
 * decodes the branch instruction then updates the PC on the machine state
 */
exec_result_t execute_branch(machine_state_t *state, decoded_instr_t instr);

#endif