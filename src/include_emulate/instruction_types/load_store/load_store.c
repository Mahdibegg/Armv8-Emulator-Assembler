#include "load_store.h"
#include "bit_utils/bit.h"

// register field value 11111 (31) encodes the zero register
#define ZERO_REGISTER 0x1F

/*
 
decode_load_store builds the ls_instr_fields struct for execution
 
bit 31 distinguishes single data transfer (1) from load literal (0).
for single data transfer, the U flag and bits 21 and 11 select the
addressing mode and which of the offset fields are valid
 
*/
 
static ls_instr_fields_t decode_load_store(decoded_instr_t instr) {
 
    word_t w = instr.instr;
 
    // struct to return; sf and rt are common to both forms
    ls_instr_fields_t fields = {
        .sf = extract_bits(w, 30, 30),
        .rt = extract_bits(w, 0, 4)
    };
 
    if (extract_bits(w, 31, 31) == 1) {
 
        // single data transfer: L selects load/store, xn is the base register
        fields.L = extract_bits(w, 22, 22);
        fields.xn = extract_bits(w, 5, 9);
 
        bit_t u_flag = extract_bits(w, 24, 24);
 
        if (u_flag == 1) {
 
            // unsigned immediate offset (the whole offset field is imm12)
            fields.type = LS_UNSIGNED_OFFSET;
            fields.imm12 = extract_bits(w, 10, 21);
        } else if (extract_bits(w, 21, 21) == 1) {
 
            // register offset; the offset register is always an X-register
            fields.type = LS_REGISTER_OFFSET;
            fields.xm = extract_bits(w, 16, 20);
        } else {
 
            // pre/post index; the I bit (11) selects which, simm9 is the offset
            fields.simm9 = extract_bits(w, 12, 20);
            fields.type = (extract_bits(w, 11, 11) == 1) ? LS_PRE_INDEX : LS_POST_INDEX;
        }
    } else {
 
        // load literal: always a load, offset is relative to the PC
        fields.type = LS_LOAD_LITERAL;
        fields.L = 1;
        fields.simm19 = extract_bits(w, 5, 23);
    }
 
    return fields;
}

/*

read_reg reads a general register, returning 0 for the zero register
(index 31, which is ZR here, not SP as not in spec)

*/

static dword_t read_reg(const machine_state_t *state, byte_t index, bit_t sf) {

    // index 31 reads as the zero register
    if (index == ZERO_REGISTER) {
        return sf ? read_xzr() : read_wzr();
    }

    // otherwise read as a 64-bit X or 32-bit W register
    return sf ? read_x_register(&state->general_registers, index)
              : read_w_register(&state->general_registers, index);
}

/*

write_reg writes a general register, discarding writes to the zero
register (index 31); a W write zero-extends the upper 32 bits

*/

static void write_reg(machine_state_t *state, byte_t index, bit_t sf, dword_t value) {

    // writes to the zero register are ignored
    if (index == ZERO_REGISTER) {
        return;
    }

    // 64-bit X write, or 32-bit W write (write_w_register zero-extends)
    if (sf) {
        write_x_register(&state->general_registers, index, value);
    } else {
        write_w_register(&state->general_registers, index, (word_t) value);
    }
}

exec_result_t execute_load_store(machine_state_t *state, decoded_instr_t instr) {
    return EXEC_NEXT;
}