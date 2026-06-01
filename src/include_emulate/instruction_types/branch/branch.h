#ifndef BRANCH_H
#define BRANCH_H

#include "state.h"
#include "pipeline/decode_struct.h"
#include "types.h"

// Unconditional Branch: b
typedef struct {
    int64_t offset;
} uncond_branch_t;

// Conditional Branch: b.cond
typedef struct {
    byte_t cond;
    int64_t offset;
} cond_branch_t;

// Register Branch: br
typedef struct {
    byte_t xn;
} reg_branch_t;

exec_result_t execute_branch(machine_state_t *state, decoded_instr_t instr);

#endif