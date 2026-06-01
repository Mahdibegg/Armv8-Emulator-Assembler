#include <stdio.h>
#include <stdlib.h>

#include "data_processing.h"
#include "bit_utils/bit.h"

// extracts bits to create immediate instruction fields as struct
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

        // for arithmetic instruction case, fill in sh, imm12, rn fields
        fields.sh = extract_bits(instr.instr, 22, 22);
        fields.imm12 = extract_bits(instr.instr, 10, 21);
        fields.rn = extract_bits(instr.instr, 5, 9);
    } else if (fields.opi == WIDE_MOVE_OPI) {

        // for wide move case, fill in hw, imm16 fields
        fields.hw = extract_bits(instr.instr, 22, 22);
        fields.imm16 = extract_bits(instr.instr, 5, 20);
    } else {

        // hand error when opi does not fit arithmetic or wide move
        fprintf(stderr, "Invalid data processing immediate instruction: unsupported opi=%u (0x%x) in instruction 0x%08x\n",
            fields.opi,
            fields.opi,
            instr.instr
        );
        exit(EXIT_FAILURE);
    }

    return fields;
}

// extract bits to create register instruction fields as struct
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

        // arithmetic/logic overlap fields - opr_MSB already set
        fields.shift = extract_bits(instr.instr, 22, 23);
        if (fields.opr_MSB == 0) {

            // setting N fields (for negation)
            fields.N = extract_bits(instr.instr, 24, 24);
        }
    } else if (fields.M == 1 && fields.opr == MULTIPLY_OPR){

        // multiplication
        fields.x = extract_bits(instr.instr, 16, 16);
        fields.ra = extract_bits(instr.instr, 10,15);
    } else {
      
        // hand error when 
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

// executing fully decoded immediate instruction (state updated)
static void execute_imm_instr(machine_state_t *state, imm_instr_fields_t fields);

//  executing fully decoded register instruction (state updated)
static void execute_reg_instr(machine_state_t *state, reg_instr_fields_t fields);

exec_result_t execute_data_processing(machine_state_t *state, decoded_instr_t instr) {

    // distinguish between immediate and register instruction
    if (instr.type == INSTR_DP_IMM) {

        // execute decoded immediate instruction fields (update state)
        execute_imm_instr(state, decode_imm_instr(instr));
    } else if (instr.type == INSTR_DP_REG) {

        // execute decoded register instruction fields (update state)
        execute_reg_instr(state, decode_reg_instr(instr));
    } else {

        // error message for unsupported operation
        fprintf(stderr, "Invalid data processing instruction: unsupported instruction type (non-immedate and non-register) 0x%x", instr);
        exit(EXIT_FAILURE);
    }

    // no branching, continue to next instruction in pipeline
    return EXEC_NEXT;
}