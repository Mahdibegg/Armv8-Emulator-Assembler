#ifndef INSTRUCTION_FIELDS_H
#define INSTRUCTION_FIELDS_H

#include "types.h"

// Single data transfer addressing modes and the load literal form
// single data transfer (bit 31 == 1) resolves to one of the first four,
// load literal (bit 31 == 0) is its own mode and is always a load
typedef enum {
    LS_UNSIGNED_OFFSET,
    LS_PRE_INDEX,
    LS_POST_INDEX,
    LS_REGISTER_OFFSET,
    LS_LOAD_LITERAL
} load_store_type_t;

// Load/store instruction fields as a struct
// Should be returned by the load/store decoder
typedef struct {

    // addressing mode (selects which fields below are valid)
    load_store_type_t type;

    // general format
    bit_t sf;       // 0 -> 32-bit Wt (4 bytes), 1 -> 64-bit Xt (8 bytes)
    bit_t L;        // 1 -> load, 0 -> store (load literal is always a load)
    byte_t rt;      // target register

    // single data transfer base register (always a 64-bit X-register)
    byte_t xn;

    // unsigned offset operand
    word_t imm12;

    // pre/post index operand (raw 9 bits, sign-extended at use)
    word_t simm9;

    // register offset operand (always a 64-bit X-register)
    byte_t xm;

    // load literal operand (raw 19 bits, sign-extended at use)
    word_t simm19;
} ls_instr_fields_t;

// Unconditional Branch: b
typedef struct {
    int64_t offset;
} uncond_branch_t;

#endif