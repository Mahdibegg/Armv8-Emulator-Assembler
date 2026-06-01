#include "branch.h"
#include <stdbool.h>
#include "include_emulate/bit_utils/bit.h"


// 3 helper functions to determine the type of the branch instruction

static bool is_unconditional(decoded_instr_t instr) {
    // Bits 26-31 of encoded instruction are 000101
    // we can shift 26 bits to the right and check if the value is 000101 = 0x5
    // we are also assuming that the instruction is a branch instruction as this will be taken care of in the execute branch function 

    // get the raw instruction
    instr_t raw = instr.instr;

    // check raw instruction sfhited 26 bits to the right
    return (raw >> 26) == 0x5;
}

static bool is_conditional(decoded_instr_t instr) {
    // Bits 31-24 of encoded instruction are 01010100 = 0x54
    // Shift 24 bits to the right and check the value 

    // get the raw instruction
    instr_t raw = instr.instr;

    // check raw instructoin shifted 24 bits to the right
    return (raw >> 24) == 0x54;
}

static bool is_reg_branch(decoded_instr_t instr) {
    // bits 31-10 of encoded instructiona are 1 1 0 1 0 1 1 0 0 0 0 1 1 1 1 1 0 0 0 0 0 0 = 0x3587C0
    // shift 10 bits to the rigth and check the value 

    instr_t raw = instr.instr;

    // check the raw instruction shifted 10 bits to the right
    return (raw >> 10) == 0x3587C0;
}

// 3 functions to extract the fields from each instruction (decoding)
// Decode Unconditional Branch Helper Function
static uncond_branch_t decode_uncond_branch( decoded_instr_t instr) {

    uncond_branch_t branch;

    instr_t raw = instr.instr;

    // extract the unsigned representation of the offset
    dword_t simm26 = extract_bits(raw, 0, 25);

    // offset - sign extend simm26 to 64 bit and multiply by 4 (or shift by 2 to the right)
    branch.offset = sign_extend(simm26, 26) << 2;

    return branch;
}

// Decode Conditional Branch Helper Function
static cond_branch_t decode_cond_branch( decoded_instr_t instr) {

    cond_branch_t branch;

    instr_t raw = instr.instr;

    // extract the first 4 bits (represents the condition)
    branch.cond = extract_bits(raw, 0, 3);

    // extract the unsigned representation of the offset
    dword_t simm19 = extract_bits(raw, 5, 23);


    // offset - sign extend simm19 to 64 bit and multiply by 4 (or shift by 2 to the right
    branch.offset = sign_extend(simm19, 19) << 2;

    return branch;
}

// Decode Register Branch Helper Function
static reg_branch_t decode_reg_branch( decoded_instr_t instr) {

    reg_branch_t branch;

    instr_t raw = instr.instr;

    // extract the bits that represent the register number 0-31
    branch.xn = extract_bits(raw, 5, 9);

    return branch;
}