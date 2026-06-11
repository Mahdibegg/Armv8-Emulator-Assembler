#ifndef BRANCH_H
#define BRANCH_H

#include "emulate/state.h"
#include "emulate/decode_struct.h"
#include "shared/types.h"
#include <stdint.h>

// Unconditional Branch: b
typedef struct {
    int64_t offset;
} uncond_branch_t;

// Conditional Branch: b.cond
typedef struct {
    unsigned cond;
    int64_t offset;
} cond_branch_t;

// Register Branch: br
typedef struct {
    unsigned xn;
} reg_branch_t;

// Used in Pipeline to update the PC value appropriately
exec_result_t execute_branch(machine_state_t *state, decoded_instr_t instr);

#endif