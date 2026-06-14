#ifndef DECODE_STRUCT_H
#define DECODE_STRUCT_H

#include "shared/types.h"

/* Instruction set modelled as enums */
typedef enum {
    INSTR_HALT,
    INSTR_DP_IMM,
    INSTR_DP_REG,
    INSTR_LOAD_STORE,
    INSTR_BRANCH,
    INSTR_UNKNOWN
} instr_type_t;

/* Struct for decoding phase results */
typedef struct {
    word_t instr;
    instr_type_t type;
} decoded_instr_t;

/* Struct for execute function next state decision */
typedef enum {
    EXEC_NEXT,
    EXEC_BRANCH,
    EXEC_HALT
} exec_result_t;

#endif