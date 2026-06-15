#include <stdio.h>

#include "encoder.h"
#include "shared/instruction_fields.h"
#include "shared/decode.h"

/* 
 * 
 * This struct stores data similar to a record
 * 
 * opcode: Stores the opcode string, which should be checked using lookup
 * type: Stores instruction type, needed for identifying instruction type for field builder
 * binary_encoding: Stores the binary bits of instruction opcode, used for filling in fields in field builder
 */

 typedef struct {
    const char *opcode;
    instr_type_t type;
    byte_t binary_encoding;
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
 * Looks up for opcode in opcode_map
 * Returns pointer to matching entry or NULL if not found
 */
static const opcode_entry_t *lookup_opcode(const char *opcode) {
    /* Function return type and char should both be const, lookup function will not allow changes */
    for (size_t i = 0; opcode_map[i].opcode != NULL; i++) {
        if (strcmp(opcode, opcode_map[i].opcode) == 0) {
            return &opcode_map[i];
        }
    }

    return NULL;
}

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
 * This is a function that builds a field for any instruction type
 * 
 * st: Symbol table used for lookup (only for the branching case)
 * tokens: Used so that numerical translation from operands/opcodes can be applied
 * instr_type: Obtained from the previous helper in the encode(), so the correct struct is selected from the union
 */
static instruction_fields_t *build_fields(symbol_table_t st, const tokenized_line_t tokens, opcode_entry_t *entry) {
    /* Initialising the struct */
    instruction_fields_t *field_block = malloc(sizeof(instruction_fields_t));

    if (field_block == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory to instruction fields on line %zu\n", 
            tokens.line_number
        );
        abort();
    }

    field_block->instr_type = entry->type;

    switch (entry->type) {
        case INSTR_DP_IMM:
        case INSTR_DP_REG:
        case INSTR_BRANCH:
        case INSTR_LOAD_STORE:
        case INSTR_HALT:
            field_block->fields.halt_instr = entry->binary_encoding;
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
            
            /* Alias handling function */
            // TODO --------

            /* Identify instruction type before selecting correct struct to fill fields in */
            opcode_entry_t *entry = lookup_opcode(tokens.data.instruction_data.opcode);

            /* No instruction found, must quit program */
            if (entry->type == NULL) {
                fprintf(stderr, "ERROR: Unknown opcode '%s' on line %zu\n",
                    tokens.data.instruction_data.opcode, 
                    tokens.line_number
                );
                abort();
            }

            /* Build field for correct struct */
            instruction_fields_t *fields = build_fields(st, tokens, entry);
        
            /* Assemble the bits from field_builder */
            // TODO ----------

            /* Take build field result to re-assign encoded_value using an instruction_assembler */
            break;
    }    

    return encoded_value;
}