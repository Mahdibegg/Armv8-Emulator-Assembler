#include <stdio.h>

#include "encoder.h"
#include "shared/instruction_fields.h"
#include "shared/decode.h"

/* This struct stores data similar to a record */
typedef struct {
    const char *opcode;
    instr_type_t type;
    byte_t encoding;
} opcode_entry_t;

/*
 * This array uses the record-like struct to map to the relative opcode byte and instruction type
 * Hence a simple lookup loop will keep the table simple
 * Aliases are not accounted for here
 */
static const opcode_entry_t opcode_map[] = {
    /* Special */
    {"halt", INSTR_HALT, 0x00},

    /* Data Processing - Immediate */
    {"add", INSTR_DP_IMM, 0x0},
    {"adds", INSTR_DP_IMM, 0x1},
    {"sub", INSTR_DP_IMM, 0x2},
    {"subs", INSTR_DP_IMM, 0x3},

    {"movn", INSTR_DP_IMM, 0x0},
    {"movz", INSTR_DP_IMM, 0x2},
    {"movk", INSTR_DP_IMM, 0x3},

    /* Data Processing - Register */
    {"add", INSTR_DP_REG, 0x0},
    {"adds", INSTR_DP_REG, 0x1},
    {"sub", INSTR_DP_REG, 0x2},
    {"subs", INSTR_DP_REG, 0x3},

    {"and", INSTR_DP_REG, 0x0},
    {"bic", INSTR_DP_REG, 0x1},
    {"orr", INSTR_DP_REG, 0x2},
    {"orn", INSTR_DP_REG, 0x3},
    {"eor", INSTR_DP_REG, 0x4},
    {"eon", INSTR_DP_REG, 0x5},
    {"ands", INSTR_DP_REG, 0x6},
    {"bics", INSTR_DP_REG, 0x7},

    {"madd", INSTR_DP_REG, 0x0},
    {"msub", INSTR_DP_REG, 0x1},

    /* Load store */
    {"ldr", INSTR_LOAD_STORE, 0x1},
    {"str", INSTR_LOAD_STORE, 0x0},

    /* Branch */
    {"b", INSTR_BRANCH, 0x0},
    {"br", INSTR_BRANCH, 0x1},
    {"beq", INSTR_BRANCH, 0x0},
    {"bne", INSTR_BRANCH, 0x1},
    {"bge", INSTR_BRANCH, 0xA},
    {"blt", INSTR_BRANCH, 0xB},
    {"bgt", INSTR_BRANCH, 0xC},
    {"ble", INSTR_BRANCH, 0xD},
    {"bal", INSTR_BRANCH, 0xE},

    {NULL, INSTR_UNKNOWN, 0x0}
};

/*
 * This struct allows field building helper functions to generalise the instruction field return type
 *
 * instr_type is an enum from a shared header file (decode) which determine the instruction type
 * fields union consists of only one type of instruction fields at a time
 */
typedef struct {
    instr_type_t instr_type;

    union {
        imm_instr_fields_t imm_instr;
        reg_instr_fields_t reg_instr;
        reg_branch_t reg_branch;
        cond_branch_t cond_branch;
        uncond_branch_t uncond_branch;
        ls_instr_fields_t ls_instr;
        word_t halt_instr;
    } fields;
} instruction_fields_t;

/*
 * Instruction type is identified based on the opcode token from a tokenized line
 *
 * tokens: Tokens are required, so that the opcode can be checked and an instruction type is identified
 */
static instr_type_t identify_instr_type(const tokenized_line_t tokens) {
    instr_type_t instr_type = INSTR_UNKNOWN;

    /* 
    
    TODO OTHER CASES

    */

    /* 
     * Error handling earlier on in the build process
     * Eliminates error handling later on in the build process
     */
    if (instr_type == INSTR_UNKNOWN) {
        fprintf(stderr, "ERROR: Could not identify instruction type on line %zu\n",
            tokens.line_number
        );
        abort();
    }
} 

/*
 * This is a function that builds a field for any instruction type
 * 
 * st: Symbol table used for lookup (only for the branching case)
 * tokens: Used so that numerical translation from operands/opcodes can be applied
 * instr_type: Obtained from the previous helper in the encode(), so the correct struct is selected from the union
 */
static instruction_fields_t *build_fields(symbol_table_t st, const tokenized_line_t tokens, instr_type_t instr_type) {
    /* Initialising the struct */
    instruction_fields_t *field_block = malloc(sizeof(instruction_fields_t));

    if (field_block == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory to instruction fields on line %zu\n", 
            tokens.line_number
        );
        abort();
    }

    field_block->instr_type = instr_type;

    switch (instr_type) {
        case INSTR_DP_IMM:
        case INSTR_DP_REG:
        case INSTR_BRANCH:
        case INSTR_LOAD_STORE:
        case INSTR_HALT:
            field_block->fields.halt_instr = HALT_INSTR;
            break;
    }

    return field_block;
};

/*
 * Assemble_directive will assemble the directive token type
 * 
 * tokens: Returns the directive value from the struct union
 */
static word_t assemble_directive(const tokenized_line_t tokens) {
    return tokens.data.directive_data.value;
}

word_t encode(symbol_table_t st, const tokenized_line_t tokens) {
    /* Value to be written to .bin file */
    word_t encoded_value = 0;

    /* 
     * Identify DIRECTIVE, LABEL, INSTRUCTION, EMPTY token types 
     * Assembling of parsed tokens are separated, since their assembly is different
     * Before INSTRUCTIONS are assembled, they must be further parsed
     */
    switch (tokens.token_type) {
        case DIRECTIVE:
            encoded_value = assemble_directive(tokens);
            break;
        case INSTRUCTION:
            // - TODO -------------
            /* Alias handling function */

            /* Identify instruction type before selecting correct struct to fill fields in */
            instr_type_t instr_type = identify_instr_type(tokens);

            /* Build field for correct struct */
            instruction_fields_t *fields = build_fields(st, tokens, instr_type);
        
            /* Assemble the bits from field_builder */

            /* Take build field result to re-assign encoded_value using an instruction_assembler */
            break;
    }    

    return encoded_value;
}