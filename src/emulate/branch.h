#ifndef BRANCH_H
#define BRANCH_H

#include "emulate/state.h"
#include "emulate/decode_struct.h"
#include "shared/types.h"
#include "shared/instruction_fields.h"
#include <stdint.h>

// Used in Pipeline to update the PC value appropriately
exec_result_t execute_branch(machine_state_t *state, decoded_instr_t instr);

#endif