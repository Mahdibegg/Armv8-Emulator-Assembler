#include "branch.h"
#include "bit_utils/bit.h"
#include "registers/registers.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*

3 functions is_conditional, is_unconditional, is_reg_branch check the encoded
instruction pattern to identify the specific branch instruction type.

conditional version - checks the fixed 01010100 pattern in bits 31-24
unconditional version - checks the fixed 000101 pattern in bits 31-26
register version - checks the fixed register branch pattern in bits 31-10 and
the required 00000 pattern in bits 4-0

*/

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

    bool opcode_matches = (raw >> 24) == 0x54;
    bool bit_4_zero = ((raw >> 4) & 0x1) == 0;
    
    return opcode_matches && bit_4_zero;
}

static bool is_reg_branch(decoded_instr_t instr) {
    // bits 31-10 of encoded instructiona are 1 1 0 1 0 1 1 0 0 0 0 1 1 1 1 1 0 0 0 0 0 0 = 0x3587C0
    // shift 10 bits to the rigth and check the value 

    instr_t raw = instr.instr;

    bool opcode_matches = (raw >> 10) == 0x3587C0;
    bool bottom_bits_zero = (raw & 0x1F) == 0;

    return opcode_matches && bottom_bits_zero;
}

/*

3 functions decode_cond_branch, decode_uncond_branch, decode_reg_branch build
the conditional/unconditional/register branch fields for execution.

conditional version - extracts simm19 and cond, sign-extends simm19 and shifts
left by 2 to calculate the byte offset
unconditional version - extracts simm26, sign-extends it and shifts left by 2
to calculate the byte offset
register version - extracts the Xn field used as the target register for br

*/
static uncond_branch_t decode_uncond_branch( decoded_instr_t instr) {

    uncond_branch_t branch;

    instr_t raw = instr.instr;

    // extract the unsigned representation of the offset
    dword_t simm26 = extract_bits(raw, 0, 25);

    // offset - sign extend simm26 to 64 bit and multiply by 4 (or shift by 2 to the left)
    branch.offset = sign_extend(simm26, 26) << 2;

    return branch;
}

static cond_branch_t decode_cond_branch( decoded_instr_t instr) {

    cond_branch_t branch;

    instr_t raw = instr.instr;

    // extract the first 4 bits (represents the condition)
    branch.cond = extract_bits(raw, 0, 3);

    // extract the unsigned representation of the offset
    dword_t simm19 = extract_bits(raw, 5, 23);


    // offset - sign extend simm19 to 64 bit and multiply by 4 (or shift by 2 to the left)
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

/*

4 functions condition_holds, execute_conditional_branch,
execute_unconditional_branch, execute_reg_branch handle branch execution.

condition_holds - evaluates the condition code using the PSR flags
conditional version - updates PC by offset if the condition holds
unconditional version - always updates PC by offset
register version - updates PC to the address stored in register Xn

*/
static void execute_unconditional_branch(machine_state_t *state, uncond_branch_t branch) {

    // get the offset from the decoded branch
    int64_t offset = branch.offset;
    
    // read the current value of the pc 
    dword_t current_pc = read_pc(&state->special_registers);

    // then write new value to the pc (PC += offset)
    dword_t new_pc = (dword_t)((int64_t)current_pc + offset);
    write_pc(&state->special_registers, new_pc);
}

static void execute_reg_branch(machine_state_t *state, reg_branch_t branch) {

    // get the register number 
    unsigned xn = branch.xn;

    // get the target address held in that register
    reg64_t target = read_x_register(&state->general_registers, xn);

    // set pc to the new register that it needs to point to
    write_pc(&state->special_registers, target);
}

static bool condition_holds(unsigned cond,  const spec_reg *spec_regs) {
    switch(cond) {

        case 0x0: // Z==1 
            return spec_regs->psr.z_flag == true;

        case 0x1: // Z == 0
            return spec_regs->psr.z_flag == false;

        case 0xA: // N == V
            return spec_regs->psr.n_flag == spec_regs->psr.v_flag;

        case 0xB: // N != V
            return spec_regs->psr.n_flag != spec_regs->psr.v_flag;

        case 0xC: // Z == 0 and N == V
            return (spec_regs->psr.z_flag == false) && (spec_regs->psr.n_flag == spec_regs->psr.v_flag);

        case 0xD: // Z == 1 or N != V
            return (spec_regs->psr.z_flag == true) || (spec_regs->psr.n_flag != spec_regs->psr.v_flag);

        case 0xE: // Any
            return true;

        default:
            fprintf(stderr, "Invalid conditional branch: Unsupported condition code %u\n", 
                cond
            );
            exit(EXIT_FAILURE);
    }
}

static void execute_conditional_branch(machine_state_t *state, cond_branch_t branch) {

    // get the offset from the decoded branch 
    int64_t offset = branch.offset;

    // get the condition that needs to be met
    unsigned cond = branch.cond;

    //check if the condition holds
    if (condition_holds(cond, &state->special_registers)) {
        // PC = PC + offset
        dword_t current_pc = read_pc(&state->special_registers);

        dword_t new_pc = (dword_t)((int64_t)current_pc + offset);
        write_pc(&state->special_registers, new_pc);
    }
}

/*

final execute_branch function uses all helpers (the static functions) 

should be used in the execution of pipeline 

*/
exec_result_t execute_branch(machine_state_t *state, decoded_instr_t instr) {

    // check type of the instruction if the isntruction is not of type branch then error
    if (instr.type != INSTR_BRANCH) {

        fprintf(stderr, "Invalid branch instruction: unsupported instruction type %u in instruction 0x%08x\n",
            instr.type,
            instr.instr
        );
        exit(EXIT_FAILURE);
    }
    // the instruction is a branch instruction now need to check what type of branch instruction it is

    if (is_conditional(instr)) {
        
        // Decode and execute conditional branch instruction (updates pc)
        cond_branch_t branch = decode_cond_branch(instr);
        execute_conditional_branch(state, branch);
    } else if (is_unconditional(instr)) {

        // Decode and execute unconditional branch instruction (updates pc)
        uncond_branch_t branch = decode_uncond_branch(instr);
        execute_unconditional_branch(state, branch);
    } else if (is_reg_branch(instr)) {

        // Decode and execute register branch instruction (updates pc)
        reg_branch_t branch = decode_reg_branch(instr);
        execute_reg_branch(state, branch);
    } else {

        // it is an unknown branch instruction so we return an error 
        fprintf(stderr, "Invalid branch instruction: unsupported branch encoding in instruction 0x%08x\n",
            instr.instr
        );
        exit(EXIT_FAILURE);
    }
    return EXEC_BRANCH;
}