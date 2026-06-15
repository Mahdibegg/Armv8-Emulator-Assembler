#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "encoder.h"
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

/* Prevent buffer overflows when concatenating for instruction reformatting in alias handler */
#define SAFE_STRCAT(dst, src, remaining) \
    do { \
        size_t len = strlen(src); \
        if (len >= remaining) { \
            fprintf(stderr, "ERROR: Instruction buffer overflow during instruction re format\n"); \
            abort(); \
        } \
        strncat(dst, src, remaining); \
        remaining -= len; \
    } while (0)

/*
 * Opcode map + lookup section
 * 
 * Each opcode string is mapped to its respective encoding and instruction type
 */

/* 
 * This struct stores data similar to a record
 * 
 * opcode: Stores the opcode string, which should be checked using lookup
 * type: Stores instruction type, needed for identifying instruction type for field builder
 * binary_encoding: Stores the binary bits of instruction opcode, used for filling in fields in field builder
 */
typedef struct {
    const token_t opcode;
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
static const opcode_entry_t *lookup_opcode(const char *opcode, const char *last_operand) {
    /* Function return type and char should both be const, lookup function will not allow changes */
    for (size_t i = 0; opcode_map[i].opcode != NULL; i++) {
        if (strcmp(opcode, opcode_map[i].opcode) == 0) {
            if (last_operand[0] != '#' && opcode_map[i].type == INSTR_DP_IMM) {
                continue;
            }
            return &opcode_map[i];
        }
    }

    return NULL;
}

/*
 * Alias map + lookup section
 * 
 * Each alias opcode string is mapped to its respective real opcode mnemonic string
 */

/* 
 * This struct stores data similar to a record
 * 
 * instr_opcode: Stores the instruction mnemonic (ones not in opcode_map) 
 * alias_opcode: Stores the alias opcode string, which should be checked using lookup
 */
typedef struct {
    const token_t alias_opcode;
    const token_t instr_opcode;
} alias_entry_t;

/*
 * Alias tuned opcode map table
 * All aliases map to an instruction opcode that can be found in the lookup table
 */
static const alias_entry_t alias_map[] = {
    {"cmp", "subs"},
    {"cmn", "adds"},
    {"neg", "sub"},
    {"negs", "subs"},
    {"tst", "ands"},
    {"mvn", "orn"},
    {"mov", "orr"},
    {"mul", "madd"},
    {"mneg", "msub"},

    {NULL, NULL}
};

/*
 * This lookup takes the alias_opcode and finds the corresponding instr_opcode
 * Returns pointer to matching entry or NULL if not found
 */
static const alias_entry_t *lookup_alias(const char *alias_opcode) {
    /* Function return type and char should both be const, lookup function will not allow changes */
    for (size_t i = 0; alias_map[i].alias_opcode != NULL; i++) {
        if (strcmp(alias_opcode, alias_map[i].alias_opcode) == 0) {
            return &alias_map[i];
        }
    }

    return NULL;
}

/*
 * Build process section
 *
 * All functions only help to further parse instructions to finally assemble binary instruction
 * Thus everything besides encode is private in this section
 */

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
 * This function will handle all the alias cases if the lookup function returns null
 *
 * This will check a separate table for alias lookup to change tokens buffer before opcode lookup
 * then correct opcode is mapped to with the new transformed instruction
 *
 * tokens: Reference tokens so that it can be cleared and re-tokenized with the alias map
 */
static void alias_handler(tokenized_line_t *tokens) {
    /* First check if opcode in tokens is an alias to continue */
    const alias_entry_t *alias_entry = lookup_alias(tokens->data.instruction_data.opcode);

    if (alias_entry != NULL) {
        /* Building re-formatted instruction with real opcode */
        char instruction[MAX_LINE_LENGTH];
        instruction[0] = '\0'; /* Strcat requires null terminator to be used */

        /* Make operand access more easier than constant struct to union to field access */
        token_t *operands = tokens->data.instruction_data.operands;

        const char *width_suffix;

        /* 
         * Single character check, no strcmp required
         * Checking if zero register has to be x or w
         */
        if (operands[0][0] == 'x') {
            width_suffix = "x";
        } else if (operands[0][0] == 'w') {
            width_suffix = "w";
        } else {
            fprintf(stderr, "ERROR: Invalid register width in alias on line: %zu\n",
                tokens->line_number
            );
            abort();
        }

        /* Prevent buffer overflow for instruction */
        size_t remaining = MAX_LINE_LENGTH - 1;

        /* Non NULL pointer means instr_opcode is not null since map is already defined */
        SAFE_STRCAT(instruction, alias_entry->instr_opcode, remaining);
        SAFE_STRCAT(instruction, " ", remaining);

        /*
         * Reformat the instruction by concatenating to buffer
         * Bunching cases that have similar real instruction formats
         */
        if (strcmp(alias_entry->alias_opcode, "cmp") == 0 ||
            strcmp(alias_entry->alias_opcode, "cmn") == 0 ||
            strcmp(alias_entry->alias_opcode, "tst") == 0) {
            /* subs/adds/ands rzr, rn, <op2> */

            SAFE_STRCAT(instruction, width_suffix, remaining);
            SAFE_STRCAT(instruction, "zr, ", remaining);
            SAFE_STRCAT(instruction, operands[0], remaining);
            SAFE_STRCAT(instruction, ", ", remaining);
            SAFE_STRCAT(instruction, operands[1], remaining);
        } else if (strcmp(alias_entry->alias_opcode, "neg") == 0 ||
                strcmp(alias_entry->alias_opcode, "negs") == 0 ||
                strcmp(alias_entry->alias_opcode, "mvn") == 0) {
            /* sub/subs/orn rd, rzr, <op2> */

            SAFE_STRCAT(instruction, operands[0], remaining);
            SAFE_STRCAT(instruction, ", ", remaining);
            SAFE_STRCAT(instruction, width_suffix, remaining);
            SAFE_STRCAT(instruction, "zr, ", remaining);
            SAFE_STRCAT(instruction, operands[1], remaining);
        } else if (strcmp(alias_entry->alias_opcode, "mov") == 0) {
            /* orr rd, rzr, rm */

            SAFE_STRCAT(instruction, operands[0], remaining);
            SAFE_STRCAT(instruction, ", ", remaining);
            SAFE_STRCAT(instruction, width_suffix, remaining);
            SAFE_STRCAT(instruction, "zr, ", remaining);
            SAFE_STRCAT(instruction, operands[1], remaining);
        } else if (strcmp(alias_entry->alias_opcode, "mul") == 0 ||
                strcmp(alias_entry->alias_opcode, "mneg") == 0) {
            /* madd/msub rd, rn, rm, rzr */

            SAFE_STRCAT(instruction, operands[0], remaining);
            SAFE_STRCAT(instruction, ", ", remaining);
            SAFE_STRCAT(instruction, operands[1], remaining);
            SAFE_STRCAT(instruction, ", ", remaining);
            SAFE_STRCAT(instruction, operands[2], remaining);
            SAFE_STRCAT(instruction, ", ", remaining);
            SAFE_STRCAT(instruction, width_suffix, remaining);
            SAFE_STRCAT(instruction, "zr", remaining);
        }

        size_t line_number = tokens->line_number;

        /* Clear and re tokenize line through its reference */
        clear_tokenized_line(tokens);
        tokenize_line(tokens, instruction, line_number);
    }
}

/*
 * This is a function that builds a field for any instruction type
 * 
 * st: Symbol table used for lookup (only for the branching case)
 * tokens: Used so that numerical translation from operands/opcodes can be applied
 * instr_type: Obtained from the previous helper in the encode(), so the correct struct is selected from the union
 */
static instruction_fields_t *build_fields(const symbol_table_t st, const tokenized_line_t *tokens, const opcode_entry_t *entry, const addr_t current_addr) {    
    /* Required pre conditions in order to continue building fields, prevents incorrect final executable */
    assert(st != NULL);
    assert(tokens != NULL);
    assert(entry != NULL);
    
    /* Initialising the struct (should free this later on) */
    instruction_fields_t *fields = malloc(sizeof(instruction_fields_t));

    if (fields == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory to instruction fields on line %zu\n", 
            tokens->line_number
        );
        abort();
    }

    fields->instr_type = entry->type;

    switch (entry->type) {
        case INSTR_DP_IMM:
        case INSTR_DP_REG:
        case INSTR_BRANCH:
            /*  
             * First make sure operands size is equal to 1, otherwise abort()
             * Different structs for the three types of branching (b, br, b.<cond>) to be filled
             * However the .offset field is common to cond and uncond branch instructions
             */

            if (tokens->data.instruction_data.operand_count != 1) {
                fprintf(stderr, "ERROR: Invalid number of operands for branch b on line %zu\n",
                    tokens->line_number
                );
                abort();
            }

            /* Label string is now accessible */
            token_t label = tokens->data.instruction_data.operands[0];

            /* 
             * Symbol_table_get has its own error handling
             * If the label didn't exist it would throw the correct error message
             * So the missing label doesn't need to be handled here 
             */
            dword_t offset = current_addr - symbol_table_get(st, label);

            if (strcmp(entry->opcode, "b") == 0) {
                /* b <literal> where <literal> is an offset calculated */

            } else if (strcmp(entry->opcode, "br") == 0) {
                /* b Xn where Xn is a 64 bit register  calculated */

            } else {
                /* 
                 * b.<cond> <literal> case
                 * <literal> is the offset calculate
                 * <cond> should be the opcode for the condition selected
                 */
            }
            break;
        case INSTR_LOAD_STORE:
        case INSTR_HALT:
            /* Case is handled by default as "and" gets looked up and handled via dp_reg logical execution */
            break;

        default:
            fprintf(stderr, "ERROR: Unhandled instruction type on line %zu\n",
                tokens->line_number
            );
            abort();
    }

    return fields;
}

/*
 * This is a function that assembles a word from a general field struct
 *
 * fields: Used so that the different sections of the word can be shifted into the correct position
 * opcode: Used to know which branch struct is going to be used by string comparing
 */
static word_t assemble_fields(instruction_fields_t *fields, const opcode_entry_t *entry, size_t line_number) {

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

                default:
                    fprintf(stderr, "ERROR: Unknown load/store type on line %zu\n",
                        line_number);
                    abort();
            }

            return instr;
        }

        case INSTR_HALT:
            /* 
             * Case is handled by default as "and" gets looked up and handled via dp_reg logical execution
             * For safety we can just also return the halt instruction
             */
            fields->fields.halt_instr = HALT_INSTR;
            return HALT_INSTR;
            break;

        default:
            fprintf(stderr, "ERROR: Unhandled instruction type on line %zu\n",
                line_number
            );
            abort();
    }
}

/*
 * Assemble_directive will assemble the directive token type
 * 
 * tokens: Returns the directive value from the struct union
 */
static word_t assemble_directive(const tokenized_line_t *tokens) {
    return tokens->data.directive_data.value;
}

word_t encode(const symbol_table_t st, tokenized_line_t *tokens, const addr_t current_addr) {
    /* Value to be written to .bin file */
    word_t encoded_value = 0;

    /* 
     * Identify DIRECTIVE, INSTRUCTION token types
     * Assembling of parsed tokens are separated, since their assembly is different
     * Before INSTRUCTIONS are assembled, they must be further parsed
     */
    switch (tokens->token_type) {
        case DIRECTIVE:
            encoded_value = assemble_directive(tokens);
            break;
        case INSTRUCTION:
            /*
             * Alias handling before opcode lookup (will update tokens buffer)
             * Tokens is NOT const only for alias_handler
             */
            alias_handler(tokens);

            /* Identify instruction type before selecting correct struct to fill fields in */
            char *last_operand = tokens->data.instruction_data.operands[tokens->data.instruction_data.operand_count -1];
            const opcode_entry_t *entry = lookup_opcode(tokens->data.instruction_data.opcode, last_operand);

            /* No instruction found, must quit program */
            if (entry == NULL) {
                
                fprintf(stderr, "ERROR: Unknown opcode '%s' on line %zu\n",
                    tokens->data.instruction_data.opcode, 
                    tokens->line_number
                );
                abort();
            }

            /* Build field for correct struct */
            instruction_fields_t *fields = build_fields(st, tokens, entry, current_addr);
        
            /* Assemble the bits from field_builder */
            encoded_value = assemble_fields(fields, entry, tokens->line_number);

            free(fields);

            /* Take build field result to re-assign encoded_value using an instruction_assembler */
            break;

        case EMPTY:
        case LABEL:
            /* 
             * Labels and empty spaces produce no output in second pass
             * If they are going to become encoded these cases should cause an error 
             * Otherwise the binary will give unexpected outputs if something is returned
             */
        default:
            fprintf(stderr, "ERROR: Unexpected token type to be encoded on line %zu\n",
                tokens->line_number);
            abort();
    }    

    return encoded_value;
}