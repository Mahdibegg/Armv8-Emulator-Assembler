#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "assemble/encoder.h"
#include "shared/instruction_fields.h"
#include "shared/decode.h"
#include "shared/shared_opcodes.h"

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
#define LS_FIXED_MID 0x1C
#define LS_LITERAL_FIXED 0x18
#define LS_UNSIGNED_U 0x1
#define LS_PRE_INDEX_MODE 0x3
#define LS_POST_INDEX_MODE 0x1
#define LS_REGISTER_OFFSET_BIT 0x1
#define LS_REGISTER_OFFSET_MODE 0x1A
#define LS_REGISTER_OFFSET_BIT_SHIFT 21

#define BR_UNCOND_FIXED 0x5
#define BR_REG_FIXED 0xD61F0000
#define BR_COND_FIXED 0x54

/*
 * Fixed opi for data processing immediate
 */
#define WIDE_MOVE_INSTR_OPI 0x5
#define ARITHMETIC_INSTR_OPI 0x2

/* Fixed bits */
#define REG_MULTIPLY_OPR 0x8
#define MULTIPLY_X_BIT 0x20

#define REG_ARITHMETIC_OPR 0x8
#define REG_LOGICAL_OPR 0x0
#define REG_LOGICAL_N_BIT 0x1

/* Shift related bits */
#define REG_SHIFT_ENCODING_SHIFT 1
#define REG_SHIFT_MAX 31
#define REG_SHIFT_MAX_64 63

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
 * Helper functions declared at top after spotting redundant code
 */

/* Remove redundant code that checks for arithmetic instruction */
static bool check_arith_instr(const char *opcode) {
    if (strcmp(opcode, "add") == 0 ||
        strcmp(opcode, "adds") == 0 ||
        strcmp(opcode, "sub") == 0 ||
        strcmp(opcode, "subs") == 0) {
            return true;
    }
    return false;
} 

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
static const opcode_entry_t *lookup_opcode(const char *opcode, const tokens_t operands, size_t operand_count) {
    for (size_t i = 0; opcode_map[i].opcode != NULL; i++) {
        if (strcmp(opcode, opcode_map[i].opcode) == 0) {

            /* Checking the arithmetic instructions and if their operand sizes are above 3 */
            if (check_arith_instr(opcode) && operand_count >= 3) {

                if (operands[2][0] != '#' && opcode_map[i].type == INSTR_DP_IMM) {
                    continue;
                } 
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

/* Take register operand and return the register number as unsigned */
static unsigned parse_reg(token_t operand, size_t line_number) {
    unsigned reg = 0;

    if (strcmp(operand, "xzr") == 0 || strcmp(operand, "wzr") == 0) {
        return 31;
    }

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

/*Take immediate operand and return the value as signed integer */
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

    /* Oversized token length handling */
    if (strlen(src) >= MAX_TOKEN_LENGTH) {
        fprintf(stderr, "ERROR: Token too long on line %zu\n", line_number);
        abort();
    }

    strcpy(dest, src);
}

/* Calculate the raw encoded offset between two byte addresses, since we don't want byte size and raw size */
static sdword_t get_offset(addr_t target_addr, addr_t current_addr) {
    return ((sdword_t) target_addr - (sdword_t) current_addr) / 4;
}

/* Parse register shift and return the shift bits for opr using string pattern match */
static byte_t parse_reg_shift(token_t shift_operand, register_type_t type, size_t line_number) {
    assert(shift_operand != NULL);

    /* 
     * Pattern match shift string and return opcode
     * Constants are defined within the shift_opcodes.h in shared
     */
    if (strcmp(shift_operand, "lsl") == 0) {
        return LSL;
    } else if (strcmp(shift_operand, "lsr") == 0) {
        return LSR;
    } else if (strcmp(shift_operand, "asr") == 0) {
        return ASR;
    } else if (strcmp(shift_operand, "ror") == 0 && type == REG_LOGIC) {
        return ROR;
    } else {
        fprintf(stderr, "ERROR: Invalid shift for register instruction on line %zu\n",
            line_number
        );
        abort();
    }
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
    const alias_entry_t *alias_entry = lookup_alias(tokens->data.instruction_data.opcode);

    if (alias_entry != NULL) {
        char instruction[MAX_LINE_LENGTH];
        instruction[0] = '\0';

        /* Smaller variable name references to operands and operand_count */
        token_t *operands = tokens->data.instruction_data.operands;
        size_t operand_count = tokens->data.instruction_data.operand_count;

        const char *width_suffix;

        /* Error handling on invalid register width */
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

        size_t remaining = MAX_LINE_LENGTH - 1;

        SAFE_STRCAT(instruction, alias_entry->instr_opcode, remaining);
        SAFE_STRCAT(instruction, " ", remaining);

        /* The nested branch checks for shifts that need to be added onto the instruction buffer */
        if (strcmp(alias_entry->alias_opcode, "cmp") == 0 ||
            strcmp(alias_entry->alias_opcode, "cmn") == 0 ||
            strcmp(alias_entry->alias_opcode, "tst") == 0) {
            /* subs/adds/ands rzr, rn, <op2> */

            SAFE_STRCAT(instruction, width_suffix, remaining);
            SAFE_STRCAT(instruction, "zr, ", remaining);
            SAFE_STRCAT(instruction, operands[0], remaining);
            SAFE_STRCAT(instruction, ", ", remaining);
            SAFE_STRCAT(instruction, operands[1], remaining);

            if (operand_count == 4) {
                SAFE_STRCAT(instruction, ", ", remaining);
                SAFE_STRCAT(instruction, operands[2], remaining);
                SAFE_STRCAT(instruction, " ", remaining);
                SAFE_STRCAT(instruction, operands[3], remaining);
            }
        } else if (strcmp(alias_entry->alias_opcode, "neg") == 0 ||
                   strcmp(alias_entry->alias_opcode, "negs") == 0 ||
                   strcmp(alias_entry->alias_opcode, "mvn") == 0) {
            /* sub/subs/orn rd, rzr, <op2> */

            SAFE_STRCAT(instruction, operands[0], remaining);
            SAFE_STRCAT(instruction, ", ", remaining);
            SAFE_STRCAT(instruction, width_suffix, remaining);
            SAFE_STRCAT(instruction, "zr, ", remaining);
            SAFE_STRCAT(instruction, operands[1], remaining);

            if (operand_count == 4) {
                SAFE_STRCAT(instruction, ", ", remaining);
                SAFE_STRCAT(instruction, operands[2], remaining);
                SAFE_STRCAT(instruction, " ", remaining);
                SAFE_STRCAT(instruction, operands[3], remaining);
            }
        } else if (strcmp(alias_entry->alias_opcode, "mov") == 0) {
            /* orr rd, rzr, rm */

            SAFE_STRCAT(instruction, operands[0], remaining);
            SAFE_STRCAT(instruction, ", ", remaining);
            SAFE_STRCAT(instruction, width_suffix, remaining);
            SAFE_STRCAT(instruction, "zr, ", remaining);
            SAFE_STRCAT(instruction, operands[1], remaining);

            if (operand_count == 4) {
                SAFE_STRCAT(instruction, ", ", remaining);
                SAFE_STRCAT(instruction, operands[2], remaining);
                SAFE_STRCAT(instruction, " ", remaining);
                SAFE_STRCAT(instruction, operands[3], remaining);
            }
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
        case INSTR_DP_IMM: {
            /*
             * First validate operand size based on entry string being a wide move or arithmetic
             * Store field bits for overlapping/general fields amongst immediate instructions (wide move and arithmetic)
             * 
             * Operand count = 2 or 4 (for wide move immediate) then operand count = 3 or 5 (for arithmetic immediate) 
             * 
             * They should be distinguished via comparing the first three letters of the opcode to be a "mov"
             * The "mov" alias is ignored since it wouldn't reach here, but the assertion is put just in case
             * Since using the above check would then not allocate the correct fields for the "mov" alias
             * 
             * For the operand count 2, 3 for move and arithmetic respectively,
             * store field bits for specific fields (within the wide move and arithmetic set of instructions)
             * 
             * For the operand count 4, 5 for move and arithmetic respectively ,
             * validate the lsl immediate to be 12 or 0 for arithmetic
             * validate lsl immediate to be 48, 32, 16 or 0 for wide move
             * ignore the other shifts
             * 
             * mov(suffix) Rd, #<imm16> case
             * mov(suffix) Rd, #<imm16>, lsl #<shift> case
             * <arithmetic_opcode> Rd, Rn, #<imm12> case
             * <arithmetic_opcode> Rd, Rn, #<imm12>, lsl #12 case
             */

            assert(strcmp(tokens->data.instruction_data.opcode, "mov") != 0);

            size_t operand_counts = tokens->data.instruction_data.operand_count;

            /* Set opcode field by default */
            fields->fields.imm_instr.opc = entry->binary_encoding;

            /* In order to compare for move you have to obtain the first 3 characters, hence strncmp is best suited */
            if (strncmp(entry->opcode, "mov", 3) == 0) {
                if (operand_counts != 2 && operand_counts != 4) {
                    fprintf(stderr, "ERROR: Invalid number of operands for wide move on line %zu\n",
                        tokens->line_number
                    );
                    abort();
                }

                /* 
                 * mov(suffix) <rd> <imm_operand> <shift opc> <shift imm>
                 * rd, imm operands are the same field in either case
                 */
                token_t rd_operand = tokens->data.instruction_data.operands[0];
                token_t imm_operand = tokens->data.instruction_data.operands[1];

                fields->fields.imm_instr.type = IMM_WIDE_MOVE;

                /* Setting general fields, <opi> and <rd> */
                fields->fields.imm_instr.opi = WIDE_MOVE_INSTR_OPI;
                fields->fields.imm_instr.rd = parse_reg(rd_operand, tokens->line_number);

                /* Distinguish 32 and 64 bit operation via sf bit */
                fields->fields.imm_instr.sf = (rd_operand[0] == 'x') ? 1 : 0;

                sdword_t imm = parse_imm(imm_operand, tokens->line_number);

                if (imm < 0 || imm > IMM16_MASK) {
                    fprintf(stderr, "ERROR: Invalid wide move immediate on line %zu\n",
                        tokens->line_number
                    );
                    abort();
                }

                /* Validated imm field can be set and .hw to prevent uninitialised field from encoding */
                fields->fields.imm_instr.imm16 = imm;
                fields->fields.imm_instr.hw = 0;

                if (operand_counts == 4) {
                    token_t shift_operand = tokens->data.instruction_data.operands[2];
                    token_t shift_amount_operand = tokens->data.instruction_data.operands[3];

                    if (strcmp(shift_operand, "lsl") != 0) {
                        fprintf(stderr, "ERROR: Invalid shift for wide move on line %zu\n",
                            tokens->line_number
                        );
                        abort();
                    }

                    sdword_t shift_amount = parse_imm(shift_amount_operand, tokens->line_number);

                    if (shift_amount % 16 != 0 || shift_amount < 0 || shift_amount > 48) {
                        fprintf(stderr, "ERROR: Invalid wide move shift amount on line %zu\n",
                            tokens->line_number
                        );
                        abort();
                    }

                    if (fields->fields.imm_instr.sf == 0 && shift_amount > 16) {
                        fprintf(stderr, "ERROR: Invalid 32-bit wide move shift amount on line %zu\n",
                            tokens->line_number
                        );
                        abort();
                    }

                    fields->fields.imm_instr.hw = shift_amount / 16;
                }

            } else {
                /* Arithmetic immediate instructions can only have either 3 or 5 operands */
                if (operand_counts != 3 && operand_counts != 5) {
                    fprintf(stderr, "ERROR: Invalid number of operands for immediate arithmetic on line %zu\n",
                        tokens->line_number
                    );
                    abort();
                }

                /* 
                 * <arithmetic opcode> <rd> <rn> <imm_operand> <shift opc> <shift imm>
                 * rd, rn and imm operands are the same field in either case
                 */
                token_t rd_operand = tokens->data.instruction_data.operands[0];
                token_t rn_operand = tokens->data.instruction_data.operands[1];
                token_t imm_operand = tokens->data.instruction_data.operands[2];

                /* Setting general fields, <opi>, <rd>, <rn> */
                fields->fields.imm_instr.type = IMM_ARITHMETIC;
                fields->fields.imm_instr.opi = ARITHMETIC_INSTR_OPI;
                fields->fields.imm_instr.rd = parse_reg(rd_operand, tokens->line_number);
                fields->fields.imm_instr.rn = parse_reg(rn_operand, tokens->line_number);

                /* Distinguish between 32 and 64 bit operation */
                fields->fields.imm_instr.sf = (rd_operand[0] == 'x') ? 1 : 0;

                sdword_t imm = parse_imm(imm_operand, tokens->line_number);

                /* Check if size of immediate is larger than largest 12 bit value */
                if (imm < 0 || imm > IMM12_MASK) {
                    fprintf(stderr, "ERROR: Invalid arithmetic immediate on line %zu\n",
                        tokens->line_number
                    );
                    abort();
                }

                /* Set .imm12 to validated parsed immediate and by default no shift for 3 operand_count */
                fields->fields.imm_instr.imm12 = imm;
                fields->fields.imm_instr.sh = 0;

                /* This case requires checking for a lsl #12 or lsl #0 for arithmetic */
                if (operand_counts == 5) {
                    /* Obtain shorter named references to operands */
                    token_t shift_operand = tokens->data.instruction_data.operands[3];
                    token_t shift_amount_operand = tokens->data.instruction_data.operands[4];

                    /* Return error to use if they have used any other shift for immediate instruction besides lsl */
                    if (strcmp(shift_operand, "lsl") != 0) {
                        fprintf(stderr, "ERROR: Invalid shift for immediate arithmetic on line %zu\n",
                            tokens->line_number
                        );
                        abort();
                    }

                    sdword_t shift_amount = parse_imm(shift_amount_operand, tokens->line_number);

                    /* Shift amount can only be equal to 12, anything else  is not valid */
                    if (shift_amount != 12 && shift_amount != 0) {
                        fprintf(stderr, "ERROR: Invalid arithmetic immediate shift amount on line %zu\n",
                            tokens->line_number
                        );
                        abort();
                    }

                    /* 5 operand_count results in a shift happening */
                    if (shift_amount != 0) {
                        fields->fields.imm_instr.sh = 1;
                    }
                }
            }

            break;
        }

        case INSTR_DP_REG: {
            /*
             * First validate operand size based on entry string being arithmetic, logical or multiply
             * Store field bits for overlapping/general fields amongst register instructions
             * 
             * Operand count = 3 or 5 for arithmetic and logical register instructions
             * Operand count = 4 for multiply register instructions
             * 
             * <arithmetic_opcode> Rd, Rn, Rm
             * <arithmetic_opcode> Rd, Rn, Rm, <shift> #<amount>
             * <logical_opcode> Rd, Rn, Rm
             * <logical_opcode> Rd, Rn, Rm, <shift> #<amount>
             * madd Rd, Rn, Rm, Ra
             * msub Rd, Rn, Rm, Ra
             */

            size_t operand_counts = tokens->data.instruction_data.operand_count;

            /* Set opcode field by default */
            fields->fields.reg_instr.opc = entry->binary_encoding;

            if (strcmp(entry->opcode, "madd") == 0 || strcmp(entry->opcode, "msub") == 0) {
                if (operand_counts != 4) {
                    fprintf(stderr, "ERROR: Invalid number of operands for multiply on line %zu\n",
                        tokens->line_number
                    );
                    abort();
                }

                /*
                 * madd/msub <rd> <rn> <rm> <ra>
                 * rd, rn, rm and ra operands are the same field in either case
                 */
                token_t rd_operand = tokens->data.instruction_data.operands[0];
                token_t rn_operand = tokens->data.instruction_data.operands[1];
                token_t rm_operand = tokens->data.instruction_data.operands[2];
                token_t ra_operand = tokens->data.instruction_data.operands[3];

                /* Setting general fields, <rd>, <rn>, <rm>, <M> and <opr> */
                fields->fields.reg_instr.rd = parse_reg(rd_operand, tokens->line_number);
                fields->fields.reg_instr.rn = parse_reg(rn_operand, tokens->line_number);
                fields->fields.reg_instr.rm = parse_reg(rm_operand, tokens->line_number);
                fields->fields.reg_instr.M = 1;
                fields->fields.reg_instr.opr = REG_MULTIPLY_OPR;

                /* Distinguish 32 and 64 bit operation via sf bit */
                fields->fields.reg_instr.sf = (rd_operand[0] == 'x') ? 1 : 0;

                /* Store Ra inside operand field */
                fields->fields.reg_instr.operand = parse_reg(ra_operand, tokens->line_number);

                /* msub sets the multiply negate bit */
                if (strcmp(entry->opcode, "msub") == 0) {
                    fields->fields.reg_instr.operand |= MULTIPLY_X_BIT;
                }

            } else {
                /* Register arithmetic/logical instructions can only have either 3 or 5 operands */
                if (operand_counts != 3 && operand_counts != 5) {
                    fprintf(stderr, "ERROR: Invalid number of operands for register instruction on line %zu\n",
                        tokens->line_number
                    );
                    abort();
                }

                /*
                 * <opcode> <rd> <rn> <rm> <shift opc> <shift imm>
                 * rd, rn and rm operands are the same field in either case
                 */
                token_t rd_operand = tokens->data.instruction_data.operands[0];
                token_t rn_operand = tokens->data.instruction_data.operands[1];
                token_t rm_operand = tokens->data.instruction_data.operands[2];

                /* Setting general fields, <rd>, <rn>, <rm> and <M> */
                fields->fields.reg_instr.rd = parse_reg(rd_operand, tokens->line_number);
                fields->fields.reg_instr.rn = parse_reg(rn_operand, tokens->line_number);
                fields->fields.reg_instr.rm = parse_reg(rm_operand, tokens->line_number);
                fields->fields.reg_instr.M = 0;

                /* Distinguish 32 and 64 bit operation via sf bit */
                fields->fields.reg_instr.sf = (rd_operand[0] == 'x') ? 1 : 0;

                /* Set default shift amount to prevent uninitialised field from encoding */
                fields->fields.reg_instr.operand = 0;


                if (check_arith_instr(entry->opcode)) {
                    register_type_t reg_type = REG_ARITHMETIC;

                    /* Register arithmetic uses arithmetic opr format */
                    fields->fields.reg_instr.opr = REG_ARITHMETIC_OPR;

                    if (operand_counts == 5) {
                        /* <shift opc> and <shift imm> for 5 operands in arithmetic register instruction */
                        token_t shift_operand = tokens->data.instruction_data.operands[3];
                        token_t shift_amount_operand = tokens->data.instruction_data.operands[4];

                        fields->fields.reg_instr.opr |= parse_reg_shift(
                            shift_operand,
                            reg_type,
                            tokens->line_number
                        );

                        sdword_t shift_amount = parse_imm(shift_amount_operand, tokens->line_number);

                        /* User has to know if shift is too large since they made error in code */
                        if (shift_amount < 0 || shift_amount > REG_SHIFT_MAX) {
                            fprintf(stderr, "ERROR: Invalid register arithmetic shift amount on line %zu\n",
                                tokens->line_number
                            );
                            abort();
                        }

                        /* Set shift amount after size validation */
                        fields->fields.reg_instr.operand = shift_amount;
                    }

                } else {
                    register_type_t reg_type = REG_LOGIC;

                    /* Register logical uses logical opr format */
                    fields->fields.reg_instr.opr = REG_LOGICAL_OPR;

                    /* Negated logical instructions set the N bit in opr */
                    if (strcmp(entry->opcode, "bic") == 0 ||
                        strcmp(entry->opcode, "orn") == 0 ||
                        strcmp(entry->opcode, "eon") == 0 ||
                        strcmp(entry->opcode, "bics") == 0) {
                        fields->fields.reg_instr.opr |= REG_LOGICAL_N_BIT;
                    }

                    if (operand_counts == 5) {
                        /* <shift opc> and <shift imm> for 5 operands in logical register instructions */
                        token_t shift_operand = tokens->data.instruction_data.operands[3];
                        token_t shift_amount_operand = tokens->data.instruction_data.operands[4];

                        /* Using |= we are able to keep the bits that we already added to opr */
                        fields->fields.reg_instr.opr |= parse_reg_shift(
                            shift_operand,
                            reg_type,
                            tokens->line_number
                        );

                        sdword_t shift_amount = parse_imm(shift_amount_operand, tokens->line_number);

                        if (shift_amount < 0 || shift_amount > REG_SHIFT_MAX) {
                            fprintf(stderr, "ERROR: Invalid register logical shift amount on line %zu\n",
                                tokens->line_number
                            );
                            abort();
                        }

                        fields->fields.reg_instr.operand = shift_amount;
                    }
                }
            }

            break;
        }

        case INSTR_BRANCH:
            /*  
             * First make sure operands size is equal to 1, otherwise abort()
             * Different structs for the three types of branching (b, br, b.<cond>) to be filled
             * However the .offset field is common to cond and uncond branch instructions
             */

            /* Validate the number of tokenized operands for all load/store forms before referencing tokens*/
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
            /*
             * Validating operand count, storing required variables and filling in overlapping fields amongst all 6 str and 5 ldr versions
             * This case will have 6 smaller cases that need to be handled for ldr and 5 identical cases that overlap in the same branches for str
             * The cases are as follows, handled in the order stated
             * 
             * ldr Rt, <literal> case
             * ldr/str Rt, [Xn] case
             * ldr/str Rt, [Xn], #<simm9> case
             * ldr/str Rt, [Xn, #<simm9>]! case
             * ldr/str Rt, [Xn, #<imm12>] case
             * ldr/str Rt, [Xn, Xm] case
             */

            /* Validate the number of tokenized operands for all load/store forms before referencing tokens*/
            size_t operand_counts = tokens->data.instruction_data.operand_count;

            if (operand_counts < 2 || operand_counts > 3) {
                fprintf(stderr, "ERROR: Invalid number of operands for load/store instruction on line %zu\n",
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
            break;
        }
            
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
                    instr |= LS_REGISTER_OFFSET_BIT << LS_REGISTER_OFFSET_BIT_SHIFT;
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
    /* Function is adapted to handle hex properly */
    char *end = NULL;
    unsigned long value = strtoul(tokens->data.directive_data.value, &end, 0);

    if (*end != '\0') {
        fprintf(stderr, "ERROR: Invalid directive value '%s' on line %zu\n",
            tokens->data.directive_data.value,
            tokens->line_number
        );
        abort();
    }

    return (word_t) value;
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

            /* 
             * Identify instruction type before selecting correct struct to fill fields in
             * Take in operands in order to select between data process immediate/register
             */
            const opcode_entry_t *entry = lookup_opcode(
                tokens->data.instruction_data.opcode, 
                tokens->data.instruction_data.operands,
                tokens->data.instruction_data.operand_count
            );

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