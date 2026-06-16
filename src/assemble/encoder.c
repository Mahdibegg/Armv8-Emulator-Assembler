#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

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
    {"b.eq", INSTR_BRANCH, 0x0},
    {"b.ne", INSTR_BRANCH, 0x1},
    {"b.ge", INSTR_BRANCH, 0xA},
    {"b.lt", INSTR_BRANCH, 0xB},
    {"b.gt", INSTR_BRANCH, 0xC},
    {"b.le", INSTR_BRANCH, 0xD},
    {"b.al", INSTR_BRANCH, 0xE},

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
 * Parse section
 * 
 * Used to take in strings and numerically interpret them in different ways
 * This section also has to do lots of syntax error handling, so user can debug syntax errors
 */

/*
 * Take register operand and return the register number as unsigned
 */
static unsigned parse_reg(token_t operand, size_t line_number) {
    unsigned reg = 0;

    if (operand == NULL) {
        fprintf(stderr, "ERROR: Missing register on line %zu\n",
            line_number
        );
        abort();
    }

    /* Check operand string starts with x or w */
    if (operand[0] != 'x' && operand[0] != 'w') {
        fprintf(stderr, "ERROR: Invalid register '%s' on line %zu\n",
            operand,
            line_number
        );
        abort();
    }

    /* Check there is a number after x or w */
    if (operand[1] == '\0') {
        fprintf(stderr, "ERROR: Missing register number on line %zu\n",
            line_number
        );
        abort();
    }

    /* Convert characters after x and w into unsigned register number */
    char *end_ptr = NULL;
    reg = (unsigned) strtoul(operand + 1, &end_ptr, 10);

    /* Check full string was valid and register is in range */
    if (*end_ptr != '\0' || reg > REG_NUM) {
        fprintf(stderr, "ERROR: Invalid register number access on line %zu\n",
            line_number
        );
        abort();
    }

    return reg;
}

/*
 * Take immediate operand and return the value as signed integer
 */
static sdword_t parse_imm(token_t operand, size_t line_number) {
    sdword_t imm = 0;

    if (operand == NULL) {
        fprintf(stderr, "ERROR: Missing immediate on line %zu\n",
            line_number
        );
        abort();
    }

    /* Check operand string starts with # for immediates */
    if (operand[0] != '#') {
        fprintf(stderr, "ERROR: Invalid immediate '%s' on line %zu\n",
            operand,
            line_number
        );
        abort();
    }

    /* Check there for number after # */
    if (operand[1] == '\0') {
        fprintf(stderr, "ERROR: Missing immediate value on line %zu\n",
            line_number
        );
        abort();
    }

    /* Convert characters after # into signed immediate value using strtoll */
    char *end_ptr = NULL;
    imm = strtoll(operand + 1, &end_ptr, 0);

    /* Check full string was valid */
    if (*end_ptr != '\0') {
        fprintf(stderr, "ERROR: Invalid immediate value '%s' on line %zu\n",
            operand,
            line_number
        );
        abort();
    }

    return imm;
}

/* Remove the first character from a string, for ldr/str parse */
static void remove_first_char(token_t str) {
    assert(str != NULL);

    memmove(str, str + 1, strlen(str));
}

/* Remove the last character from a string, for ldr/str parse */
static void remove_last_char(token_t str) {
    assert(str != NULL);

    size_t len = strlen(str);

    if (len > 0) {
        str[len - 1] = '\0';
    }
}

/* 
 * Remove [] from the start and ] from the end if they exist 
 * Used for further parse of the [x, y] in operand tokens
 */
static void remove_brackets(token_t str) {
    assert(str != NULL);

    if (str[0] == '[') {
        remove_first_char(str);
    }

    size_t len = strlen(str);

    if (len > 0 && str[len - 1] == ']') {
        remove_last_char(str);
    }
}

/* Remove ! from the end if it exists for ldr/str further parse */
static void remove_index_suffix(token_t str) {
    assert(str != NULL);

    size_t len = strlen(str);

    if (len > 0 && str[len - 1] == '!') {
        remove_last_char(str);
    }
}

/* 
 * Copy a token into a new buffer since the token buffer is constant
 * src or dest token being null means tokens are missing (if correct arguments passed)
 */
static void copy_token(token_t dest, token_t src, size_t line_number) {
    assert(dest != NULL);
    assert(src != NULL);
    assert(strlen(src) >= MAX_TOKEN_LENGTH);

    strcpy(dest, src);
}

/* Calculate the raw encoded offset between two byte addresses, since we don't want byte size and raw size */
static sdword_t get_offset(addr_t target_addr, addr_t current_addr) {
    return ((sdword_t) target_addr - (sdword_t) current_addr) / 4;
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
    /* Required pre conditions in order to continue building fields */
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

            /* 
             * The operand can either be the literal for b or b.<cond> or a register for br
             *
             * symbol_table_get function has its own error handling when called for b or b.<cond>
             * If the label didn't exist it would throw the correct error message
             * So the missing label doesn't need to be handled here 
             */
            token_t operand = tokens->data.instruction_data.operands[0];

            if (strcmp(entry->opcode, "b") == 0) {
                /* b <literal> where <literal> is an offset calculated */
                sdword_t offset = get_offset(symbol_table_get(st, operand), current_addr);
                fields->fields.uncond_branch.offset = offset;
                
            } else if (strcmp(entry->opcode, "br") == 0) {
                /* b Xn where Xn is a 64 bit register  calculated */
                fields->fields.reg_branch.xn = parse_reg(operand, tokens->line_number);

            } else {
                /* 
                 * b.<cond> <literal> case
                 * <literal> is the offset calculate
                 * <cond> should be the opcode for the condition selected
                 */
                sdword_t offset = get_offset(symbol_table_get(st, operand), current_addr);
                fields->fields.cond_branch.offset = offset;

                fields->fields.cond_branch.cond = entry->binary_encoding;
            }
            break;

        case INSTR_LOAD_STORE: {
            /* Validate the number of tokenized operands for all load/store forms before referencing tokens*/
            size_t operand_counts = tokens->data.instruction_data.operand_count;

            if (operand_counts < 2 || operand_counts > 3) {
                fprintf(stderr, "ERROR: Invalid number of operands for load/store on line %zu\n",
                    tokens->line_number
                );
                abort();
            }

            /* First operand is always the target register in W or X*/
            token_t rt_operand = tokens->data.instruction_data.operands[0];

            /* Second operand is a label or the start of the address operand */
            token_t addr_operand = tokens->data.instruction_data.operands[1];

            /* Obtain the register encoding to fill in rt field (ignore register size) */
            fields->fields.ls_instr.rt = parse_reg(rt_operand, tokens->line_number);

            /* Set sf bits based on the register size */
            fields->fields.ls_instr.sf = (rt_operand[0] == 'x') ? 1 : 0;

            /* Check if the instruction is a ldr or str */
            fields->fields.ls_instr.L = (strcmp(entry->opcode, "ldr") == 0) ? 1 : 0;

            /* Load literal uses a label instead of a bracketed address */
            if (addr_operand[0] != '[') {
                /* ldr Rt, <literal> case since the [ does not appear in the first address operands*/
                
                /* Cannot parse this format for str, user would have made a bug in assembly */
                if (strcmp(entry->opcode, "ldr") != 0) {
                    fprintf(stderr, "ERROR: Invalid load/store literal on line %zu\n",
                        tokens->line_number
                    );
                    abort();
                }

                fields->fields.ls_instr.type = LS_LOAD_LITERAL;

                /* Store signed offset then write to the .simm19 field and scale since its a raw size and not byte size*/
                sdword_t offset = get_offset(symbol_table_get(st, addr_operand), current_addr);
                fields->fields.ls_instr.simm19 = offset;

            } else {
                /* ldr/str Rt, [Xn] case, as the [ exists in the first address operand */

                /* Initialise fixed size buffers for copying, valid operand sizes would not exceed a maximum length*/
                char xn_operand[MAX_TOKEN_LENGTH];
                char offset_operand[MAX_TOKEN_LENGTH];

                /* Copy and clean the base register operand, sinze the previous tokenize function only splits based on a , */
                copy_token(xn_operand, addr_operand, tokens->line_number);
                remove_brackets(xn_operand);

                /* Base only addressing has no offset operand */
                if (operand_counts == 2) {
                    fields->fields.ls_instr.type = LS_UNSIGNED_OFFSET;
                    fields->fields.ls_instr.xn = parse_reg(xn_operand, tokens->line_number);
                    fields->fields.ls_instr.imm12 = 0;

                } else {
                    /* Third operand contains the offset immediate or offset register, handled separately */
                    token_t raw_offset_operand = tokens->data.instruction_data.operands[2];

                    copy_token(offset_operand, raw_offset_operand, tokens->line_number);

                    /* Post index addressing keeps the closing bracket on the base operand */
                    if (addr_operand[strlen(addr_operand) - 1] == ']') {
                        /* ldr/str Rt, [Xn], #<simm9> case */
                        fields->fields.ls_instr.type = LS_POST_INDEX;
                        fields->fields.ls_instr.xn = parse_reg(xn_operand, tokens->line_number);
                        fields->fields.ls_instr.simm9 = parse_imm(offset_operand, tokens->line_number);

                    } else {
                        /* Cleaning offset operand before continuing with other formats */
                        remove_index_suffix(offset_operand);
                        remove_brackets(offset_operand);

                        
                        if (raw_offset_operand[strlen(raw_offset_operand) - 1] == '!') {
                            /* ldr/str Rt, [Xn, #<simm9>]! case */

                            /* Pre index addressing has the index suffix */
                            fields->fields.ls_instr.type = LS_PRE_INDEX;
                            fields->fields.ls_instr.xn = parse_reg(xn_operand, tokens->line_number);
                            fields->fields.ls_instr.simm9 = parse_imm(offset_operand, tokens->line_number);

                        } else if (offset_operand[0] == '#') {
                            /* ldr/str Rt, [Xn, #<imm12>] case */

                            /* Unsigned offset addressing uses an immediate offset */
                            sdword_t imm = parse_imm(offset_operand, tokens->line_number);

                            fields->fields.ls_instr.type = LS_UNSIGNED_OFFSET;
                            fields->fields.ls_instr.xn = parse_reg(xn_operand, tokens->line_number);

                            /* Store the .imm12 field by scaling down since we count a raw offset and not in bytes */
                            if (fields->fields.ls_instr.sf == 1) {
                                fields->fields.ls_instr.imm12 = imm / 8;
                            } else {
                                fields->fields.ls_instr.imm12 = imm / 4;
                            }
                        
                        } else {
                            /* ldr/str Rt, [Xn, Xm] case */
                            
                            /* Register offset addressing uses a register offset */
                            fields->fields.ls_instr.type = LS_REGISTER_OFFSET;
                            fields->fields.ls_instr.xn = parse_reg(xn_operand, tokens->line_number);
                            fields->fields.ls_instr.xm = parse_reg(offset_operand, tokens->line_number);
                        }
                    }
                }
            }
        }
            break;
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
            instr |= ((word_t) f.cond & FOUR_BIT_MASK);

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
    return atoi(tokens->data.directive_data.value);
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