#include <stdio.h>
#include <stdlib.h>

#include "data_processing.h"
#include "bit_utils/bit.h"

imm_instr_fields decode_imm_instr(decoded_instr_t instr) {

    // struct to return
    imm_instr_fields fields = {
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

reg_instr_fields decode_reg_instr(decoded_instr_t instr) {
    
    // struct to return
    reg_instr_fields fields = {
        .sf = extract_bits(instr.instr, 31, 31),
        .opc = extract_bits(instr.instr, 29, 30),
        .opr = extract_bits(instr.instr, 21, 24),
        .rm = extract_bits(instr.instr, 16, 20),
        .rn = extract_bits(instr.instr, 5, 9),
        .rd = extract_bits(instr.instr, 0, 4)
    };

    // differentiate between  
}

exec_result_t execute_data_processing(machine_state_t *state, decoded_instr_t instr) {

    // distinguish between immediate and register instruction
    if (instr.type == INSTR_DP_IMM) {
        decode_imm_instr(instr);
    } else if (instr.type == INSTR_DP_REG) {
        decode_reg_instr(instr);
    } else {

        // code should ideally be unreachable, just for safety

        fprintf(stderr, "Invalid operation: non-immediate/register data process not supported");
        exit(EXIT_FAILURE);
    }

    // no branching, continue to next instruction in pipeline
    return EXEC_NEXT;
}