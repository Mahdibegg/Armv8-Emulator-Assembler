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
