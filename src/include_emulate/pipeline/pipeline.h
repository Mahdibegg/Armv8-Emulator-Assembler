#ifndef PIPELINE_H
#define PIPELINE_H

#include "types.h"
#include "state.h"

// instruction set modelled as enums
typedef enum {
    INSTR_HALT,
    INSTR_DP_IMM,
    INSTR_DP_REG,
    INSTR_LOAD_STORE,
    INSTR_BRANCH,
    INSTR_UNKNOWN
} instr_type_t;

// struct for decoding phase results
typedef struct {
    word_t instr;
    instr_type_t type;
} decoded_instr_t;

// struct for execute function next state decision
typedef enum { 
    EXEC_NEXT,
    EXEC_BRANCH,
    EXEC_HALT
} exec_result_t;

// fetch instruction only reads (typealias for memory reading)
word_t fetch_instr(const machine_state_t *state);

// result of fetch_instr passed for decoding
decoded_instr_t decode_instr(const word_t instr);

// state will be updated, hence it's a pointer
exec_result_t execute_instr(machine_state_t *state, const decoded_instr_t instruction);

#endif