#include <stdio.h>
#include <stdlib.h>

#include "data_processing.h"
#include "bit_utils/bit.h"

/*

2 functions decode_imm_instr, decode_reg_instr build the immediate/register instr_fields for execution

immediate version - using the opi field against hardcoded constants (in data_processing.h) to set type and operand fields
register version - checking M, OPR's MSB and LSB and OPR to determine type field

*/

static imm_instr_fields_t decode_imm_instr(decoded_instr_t instr) {

    // struct to return
    imm_instr_fields_t fields = {
        .sf = extract_bits(instr.instr, 31, 31),
        .opc = extract_bits(instr.instr, 29, 30),
        .opi = extract_bits(instr.instr, 23, 25),
        .rd = extract_bits(instr.instr, 0, 4)
    };

    // differentiate between arithmetic/wide move operand
    if (fields.opi == ARITHMETIC_OPI) {

        // update type to now arithmetic
        fields.type = IMM_ARITHMETIC;

        // for arithmetic instruction case, fill in sh, imm12, rn fields, ignore hw, imm16
        fields.sh = extract_bits(instr.instr, 22, 22);
        fields.imm12 = extract_bits(instr.instr, 10, 21);
        fields.rn = extract_bits(instr.instr, 5, 9);
    } else if (fields.opi == WIDE_MOVE_OPI) {

        // update type to now wide move
        fields.type = IMM_WIDE_MOVE;

        // for wide move case, fill in hw, imm16 fields, ignore sh, imm12, rn fields
        fields.hw = extract_bits(instr.instr, 22, 22);
        fields.imm16 = extract_bits(instr.instr, 5, 20);
    } else {

        // handle error when opi does not fit arithmetic or wide move
        // emulator does not support any other case and unknown opi
        fprintf(stderr, "Invalid data processing immediate instruction: unsupported opi=%u (0x%x) in instruction 0x%08x\n",
            fields.opi,
            fields.opi,
            instr.instr
        );
        exit(EXIT_FAILURE);
    }

    return fields;
}

static reg_instr_fields_t decode_reg_instr(decoded_instr_t instr) {
    
    // struct to return
    reg_instr_fields_t fields = {
        .sf = extract_bits(instr.instr, 31, 31),
        .opc = extract_bits(instr.instr, 29, 30),
        .M = extract_bits(instr.instr, 28, 28),
        .opr = extract_bits(instr.instr, 21, 24),
        .opr_MSB = extract_bits(instr.instr, 24, 24),
        .rm = extract_bits(instr.instr, 16, 20),
        .rn = extract_bits(instr.instr, 5, 9),
        .rd = extract_bits(instr.instr, 0, 4)
    };

    // differentiate between arithmetic/logic and multiply
    if (fields.M == 0 && fields.opr_MSB == 0) {

        // fields.type updated to arithmetic
        fields.type = REG_ARITHMETIC;

        // arithmetic/logic overlap fields - opr_MSB already set
        fields.shift = extract_bits(instr.instr, 22, 23);
        if (fields.opr_MSB == 0) {

            // field.type updated to logic (from arithmetic)
            fields.type = REG_LOGIC;

            // setting N fields (for negation)
            fields.N = extract_bits(instr.instr, 24, 24);
        }
    } else if (fields.M == 1 && fields.opr == MULTIPLY_OPR){

        // field.type updated to multiply
        fields.type = REG_MULTIPLY;

        // multiplication extracts x and ra bits (ignoring the the shift and N fields)
        fields.x = extract_bits(instr.instr, 16, 16);
        fields.ra = extract_bits(instr.instr, 10,15);
    } else {
      
        // handling error case for any M/OPR that the emulator does not support
        fprintf(stderr, "Invalid data processing register instruction: unsupported M=%u (0x%x), opr=%u (0x%x) in instruction 0x%08x\n",
            fields.M,
            fields.M,
            fields.opr,
            fields.opr,
            instr.instr
        );
        exit(EXIT_FAILURE);  
    }

    return fields;
}

/*

5 execute functions below for the different type, each one ideally has a switch case and 

uses the desired field to do real operations that would update the state

*/

static void execute_imm_arithmetic(machine_state_t *state, imm_instr_fields_t fields, instr_t instr);

static void execute_imm_wide_move(machine_state_t *state, imm_instr_fields_t fields, instr_t instr);

static void execute_reg_arithmetic(machine_state_t *state, imm_instr_fields_t fields, instr_t instr);

static void execute_reg_logic(machine_state_t *state, imm_instr_fields_t fields, instr_t instr);

static void execute_multiply_logic(machine_state_t *state, imm_instr_fields_t fields, instr_t instr);

/*

2 higher level execution functions that group the immediate/register class of instructions

executing instructions decoded into immediate/register class

*/

static void execute_imm_instr(machine_state_t *state, imm_instr_fields_t fields, instr_t instr) {
    
    // checking type of immediate to execute it more specifically 
    // due to it having its own respective fields
    switch (fields.type) {

        // specifically execute the arithmetic instruction with sh, imm12, rn for (add, sub, adds, subs)
        case IMM_ARITHMETIC:
            void;

        // specifically execute the immediate with hw, imm16 for (movn, movz, movk)
        case IMM_WIDE_MOVE:
            void;
            
        default:

            // provide address of invalid operation if IMM_NULL and actual instruction failed to execute
            fprintf(stderr, "Invalid operation: unsupported immediate instruction (0x%x) executed at address 0x%x\n", 
                instr,
                read_pc(&state->special_registers)
            );
            exit(EXIT_FAILURE);
    }
}

static void execute_reg_instr(machine_state_t *state, reg_instr_fields_t fields, instr_t instr) {

    // checking type of register to execute it more specifically 
    // due to it having its own respective fields
    switch (fields.type) {

        // execute arithmetic shift using the shift field for (lsl, lsr, asr, ror)
        case REG_ARITHMETIC:
            void;

        // using logical shift and N field for executing
        // (and, bic, orr, orn, eor, eon, ands, bics)
        case REG_LOGIC:
            void;

        // using the x field for executing (madd, msub)
        // then using the ra field as a third input register for multiply instructions
        case REG_MULTIPLY:
            void;

        default:

            // provide address of invalid operation if IMM_NULL or
            // non immedate instruction is attempted to be executed
            fprintf(stderr, "Invalid operation: unsupported immediate execution at address 0x%x\n",
                read_pc(&state->special_registers)
            );
            exit(EXIT_FAILURE);
    }
}

/*

final execute_data_processing function puts all helpers (the static functions) 

should be used in the execution of pipeline 

*/

exec_result_t execute_data_processing(machine_state_t *state, decoded_instr_t instr) {

    // distinguish between immediate and register instruction so it can further decode
    // the correct type
    if (instr.type == INSTR_DP_IMM) {

        // pass further decoded result into execution straight away along with state pointer
        execute_imm_instr(state, decode_imm_instr(instr), instr.instr);
    } else if (instr.type == INSTR_DP_REG) {

        // similar as above but with reg version
        execute_reg_instr(state, decode_reg_instr(instr), instr.instr);
    } else {

        // error message for unsupported other forms of data_processing (or branching/load_store)
        fprintf(stderr, "Invalid data processing instruction: unsupported instruction type (non-immedate and non-register) 0x%x", instr);
        exit(EXIT_FAILURE);
    }

    // no branching, continue to next instruction in pipeline
    // PC should be updated outside loop
    return EXEC_NEXT;
}