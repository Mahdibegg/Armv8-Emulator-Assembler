#ifndef BRANCH_H
#define BRANCH_H

#include "state.h"
#include "pipeline/decode_struct.h"
#include "types.h"


/*
Unconditional Branch: b
contains:
a signed 26 bit immediate which is the offset / 4
the actual offset is sign extended and multiplied by 4 
*/
typedef struct {

    int64_t offset;

} uncond_branch_t;


/*
Conditional Branch: b.cond
contains:
4 bit ecoding of the condition that must be satisfied for the branch to be executed
signed 19 bit immediate which is the offset / 4
actual offset is sign extended and multiplied by 4
*/

typedef struct {
    
    byte_t cond;

    int64_t offset;

} cond_branch_t;


/*
Register Branch: br
contains:
5 bit unsigned register number (0-31)
*/

typedef struct {

    byte_t xn;

} reg_branch_t;


exec_result_t execute_branch(machine_state_t *state, decoded_instr_t instr);

#endif