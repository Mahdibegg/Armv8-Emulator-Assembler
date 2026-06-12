#include "tokenizer.h"
#include "../shared/types.h"

/*
 * This enum is for distinguishing the token type
 * Useful for later on when filling in separate tokens based on the type
 */
typedef enum {INSTRUCTION, DIRECTIVE, LABEL} tokenized_type_t;

/* 
 * Tokenized_instr_t is a struct that sums up all the tokens and has a token type
 * Token_type identifies which type the token is for further parsing to be made easier 
 * Label is the token that belongs to a LABEL type
 * Opcode belongs to DIRECTIVE and INSTRUCTION token_types
 * Operands belongs to the INSTRUCTION token_types, which is a list of operands the instruction takes
 */
struct tokenized_line_t {
    /* Identify token type  */
    tokenized_type_t token_type;
    
    /* Label tokens if token_type is a LABEL */
    token_t label;

    /* Instruction tokens */
    token_t opcode; /* Also belongs to directive */
    tokens_t operands;
};