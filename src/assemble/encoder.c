#include <stdio.h>

#include "assemble/encoder.h"
#include "shared/instruction_fields.h"
#include "shared/decode.h"

/* General masks for set up */
#define BIT_MASK 0x1
#define TWO_BIT_MASK 0x3
#define THREE_BIT_MASK 0x7
#define FOUR_BIT_MASK 0xF
#define FIVE_BIT_MASK 0x1F
#define SIX_BIT_MASK 0x3F
#define IMM9_MASK 0x1FF
#define IMM12_MASK 0xFFF
#define IMM16_MASK 0xFFFF
#define IMM19_MASK 0x7FFFF
#define IMM26_MASK 0x3FFFFFF

/* Field positions for shifting field bits */
#define SF_SHIFT 31
#define DP_OPC_SHIFT 29
#define DP_IMM_FIXED_SHIFT 26
#define DP_IMM_OPI_SHIFT 23
#define DP_IMM_SH_SHIFT 22
#define DP_IMM_HW_SHIFT 21
#define DP_IMM_IMM12_SHIFT 10
#define DP_IMM_IMM16_SHIFT 5
#define DP_IMM_RN_SHIFT 5

#define DP_REG_OPC_SHIFT 29
#define DP_REG_M_SHIFT 28
#define DP_REG_FIXED_SHIFT 25
#define DP_REG_OPR_SHIFT 21
#define DP_REG_RM_SHIFT 16
#define DP_REG_OPERAND_SHIFT 10
#define DP_REG_RN_SHIFT 5

#define LS_SF_SHIFT 30
#define LS_FIXED_TOP_SHIFT 31
#define LS_FIXED_MID_SHIFT 25
#define LS_U_SHIFT 24
#define LS_L_SHIFT 22
#define LS_XM_SHIFT 16
#define LS_SIMM9_SHIFT 12
#define LS_OFFSET_SHIFT 10
#define LS_XN_SHIFT 5
#define LS_LITERAL_FIXED_SHIFT 24
#define LS_LITERAL_SIMM19_SHIFT 5

#define BR_UNCOND_FIXED_SHIFT 26
#define BR_REG_XN_SHIFT 5
#define BR_COND_FIXED_SHIFT 24
#define BR_COND_OFFSET_SHIFT 5

/* 
 * Fixed bit patterns which are for the missing bits not included in field
 * These are the bits that uniquely identify the instruction
 */
#define DP_IMM_FIXED 0x4
#define DP_REG_FIXED 0x5

#define LS_FIXED_TOP 0x1
#define LS_FIXED_MID 0xC
#define LS_LITERAL_FIXED 0x18
#define LS_UNSIGNED_U 0x1
#define LS_PRE_INDEX_MODE 0x3
#define LS_POST_INDEX_MODE 0x1
#define LS_REGISTER_OFFSET_BIT 0x1
#define LS_REGISTER_OFFSET_MODE 0x1A

#define BR_UNCOND_FIXED 0x5
#define BR_REG_FIXED 0xD61F0000
#define BR_COND_FIXED 0x54

/* 
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
        // TODO ALL CASES ------------
        case INSTR_DP_IMM:
        case INSTR_DP_REG:
        case INSTR_BRANCH:
        case INSTR_LOAD_STORE:
        case INSTR_HALT:
            /* Case is handled by default as "and" gets looked up and handled via dp_reg logical execution */
            break;
    }

    return field_block;
};

/*
 * This is a function that assembles a word from a general field struct
 *
 * fields: Used so that the different sections of the word can be shifted into the correct position
 * opcode: Used to know which branch struct is going to be used by string comparing
 */
static word_t assemble_fields(instruction_fields_t *fields, const opcode_entry_t *entry) {

    /* 
     * Case checking instruction type and shifting bits into correct position using fixed constants (reduce magic number usage)
     * Using f as a copy of imm_instr reference in fields to keep each line shorter
     */

    switch (fields->instr_type) {
        case INSTR_DP_IMM: {
            imm_instr_fields_t f = fields->fields.imm_instr;

            word_t instr = 0;
            instr |= ((word_t) f.sf & BIT_MASK) << SF_SHIFT;
            instr |= ((word_t) f.opc & TWO_BIT_MASK) << DP_OPC_SHIFT;
            instr |= DP_IMM_FIXED << DP_IMM_FIXED_SHIFT;
            instr |= ((word_t) f.opi & THREE_BIT_MASK) << DP_IMM_OPI_SHIFT;
            instr |= ((word_t) f.rd & FIVE_BIT_MASK);

            if (f.type == IMM_ARITHMETIC) {
                instr |= ((word_t) f.sh & BIT_MASK) << DP_IMM_SH_SHIFT;
                instr |= ((word_t) f.imm12 & IMM12_MASK) << DP_IMM_IMM12_SHIFT;
                instr |= ((word_t) f.rn & FIVE_BIT_MASK) << DP_IMM_RN_SHIFT;
            } else if (f.type == IMM_WIDE_MOVE) {
                instr |= ((word_t) f.hw & TWO_BIT_MASK) << DP_IMM_HW_SHIFT;
                instr |= ((word_t) f.imm16 & IMM16_MASK) << DP_IMM_IMM16_SHIFT;
            }

            return instr;
        }

        case INSTR_DP_REG: {
            reg_instr_fields_t f = fields->fields.reg_instr;

            word_t instr = 0;
            instr |= ((word_t) f.sf & BIT_MASK) << SF_SHIFT;
            instr |= ((word_t) f.opc & TWO_BIT_MASK) << DP_REG_OPC_SHIFT;
            instr |= ((word_t) f.M & BIT_MASK) << DP_REG_M_SHIFT;
            instr |= DP_REG_FIXED << DP_REG_FIXED_SHIFT;
            instr |= ((word_t) f.opr & FOUR_BIT_MASK) << DP_REG_OPR_SHIFT;
            instr |= ((word_t) f.rm & FIVE_BIT_MASK) << DP_REG_RM_SHIFT;
            instr |= ((word_t) f.operand & SIX_BIT_MASK) << DP_REG_OPERAND_SHIFT;
            instr |= ((word_t) f.rn & FIVE_BIT_MASK) << DP_REG_RN_SHIFT;
            instr |= ((word_t) f.rd & FIVE_BIT_MASK);

            return instr;
        }

        case INSTR_BRANCH: {
            if (strcmp(entry->opcode, "b") == 0) {
                uncond_branch_t f = fields->fields.uncond_branch;

                word_t instr = 0;
                instr |= BR_UNCOND_FIXED << BR_UNCOND_FIXED_SHIFT;
                instr |= ((word_t) f.offset & IMM26_MASK);

                return instr;
            }

            if (strcmp(entry->opcode, "br") == 0) {
                reg_branch_t f = fields->fields.reg_branch;

                word_t instr = BR_REG_FIXED;
                instr |= ((word_t) f.xn & FIVE_BIT_MASK) << BR_REG_XN_SHIFT;

                return instr;
            }

            cond_branch_t f = fields->fields.cond_branch;

            word_t instr = 0;
            instr |= BR_COND_FIXED << BR_COND_FIXED_SHIFT;
            instr |= ((word_t) f.offset & IMM19_MASK) << BR_COND_OFFSET_SHIFT;
            instr |= ((word_t) entry->binary_encoding & FOUR_BIT_MASK);

            return instr;
        }

        case INSTR_LOAD_STORE: {
            ls_instr_fields_t f = fields->fields.ls_instr;

            if (f.type == LS_LOAD_LITERAL) {
                word_t instr = 0;
                instr |= ((word_t) f.sf & BIT_MASK) << LS_SF_SHIFT;
                instr |= LS_LITERAL_FIXED << LS_LITERAL_FIXED_SHIFT;
                instr |= ((word_t) f.simm19 & IMM19_MASK) << LS_LITERAL_SIMM19_SHIFT;
                instr |= ((word_t) f.rt & FIVE_BIT_MASK);

                return instr;
            }

            word_t instr = 0;
            instr |= LS_FIXED_TOP << LS_FIXED_TOP_SHIFT;
            instr |= ((word_t) f.sf & BIT_MASK) << LS_SF_SHIFT;
            instr |= LS_FIXED_MID << LS_FIXED_MID_SHIFT;
            instr |= ((word_t) f.L & BIT_MASK) << LS_L_SHIFT;
            instr |= ((word_t) f.xn & FIVE_BIT_MASK) << LS_XN_SHIFT;
            instr |= ((word_t) f.rt & FIVE_BIT_MASK);

            switch (f.type) {
                case LS_UNSIGNED_OFFSET:
                    instr |= LS_UNSIGNED_U << LS_U_SHIFT;
                    instr |= ((word_t) f.imm12 & IMM12_MASK) << LS_OFFSET_SHIFT;
                    break;

                case LS_PRE_INDEX:
                    instr |= ((word_t) f.simm9 & IMM9_MASK) << LS_SIMM9_SHIFT;
                    instr |= LS_PRE_INDEX_MODE << LS_OFFSET_SHIFT;
                    break;

                case LS_POST_INDEX:
                    instr |= ((word_t) f.simm9 & IMM9_MASK) << LS_SIMM9_SHIFT;
                    instr |= LS_POST_INDEX_MODE << LS_OFFSET_SHIFT;
                    break;

                case LS_REGISTER_OFFSET:
                    instr |= LS_REGISTER_OFFSET_BIT << DP_IMM_HW_SHIFT;
                    instr |= ((word_t) f.xm & FIVE_BIT_MASK) << LS_XM_SHIFT;
                    instr |= LS_REGISTER_OFFSET_MODE << LS_OFFSET_SHIFT;
                    break;

                case LS_LOAD_LITERAL:
                    break;
            }

            return instr;
        }

        case INSTR_HALT:
            /* Case is handled by default as "and" gets looked up and handled via dp_reg logical execution */
            break;
    }

    return 0;
}

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
            encoded_value = assemble_fields(fields, entry);

            /* Take build field result to re-assign encoded_value using an instruction_assembler */
            break;
    }    

    return encoded_value;
}