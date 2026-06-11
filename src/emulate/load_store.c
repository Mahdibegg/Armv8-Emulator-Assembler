#include <stdio.h>
#include <stdlib.h>

#include "emulate/load_store.h"
#include "shared/bit.h"
#include "emulate/registers.h"
#include "emulate/memory.h"

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

unsupported_load_store_error reports getting an addressing mode the
decoder shouldn't produce also shows the failing instruction address

*/

static void unsupported_load_store_error(byte_t mode, word_t address) {

    fprintf(stderr, "Invalid operation: unsupported load/store mode (0x%02x) at address 0x%08x\n",
        mode,
        address
    );
    exit(EXIT_FAILURE);
}

/*

compute_address resolves the address for a single data transfer

reads the base register Xn, then adjusts it per the addressing mode
pre/post index also write the updated address back to Xn

*/

static addr_t compute_address(machine_state_t *state, ls_instr_fields_t fields) {

    // base register, always read as a 64-bit X-register
    dword_t base = read_reg_sf(&state->general_registers, fields.xn, 1);

    switch (fields.type) {

        // unsigned offset: scale imm12 by transfer size, 8 (X) or 4 (W)
        case LS_UNSIGNED_OFFSET:
            return (addr_t) (base + (dword_t) fields.imm12 * (fields.sf ? 8 : 4));

        // register offset: add the value in Xm
        case LS_REGISTER_OFFSET:
            return (addr_t) (base + read_reg_sf(&state->general_registers, fields.xm, 1));

        // pre-index: address is base + simm9, written back before transfer
        case LS_PRE_INDEX: {
            addr_t address = (addr_t) (base + sign_extend(fields.simm9, 9));
            write_reg_sf(&state->general_registers, fields.xn, 1, address);
            return address;
        }

        // post-index: transfer at base, then Xn updated by simm9
        case LS_POST_INDEX:
            write_reg_sf(&state->general_registers, fields.xn, 1, base + sign_extend(fields.simm9, 9));
            return (addr_t) base;

        default:

            // load literal handled separately, so this should be unreachable
            unsupported_load_store_error(fields.type, read_pc(&state->special_registers));
            return 0;
    }
}

/*

execute_load_store carries out one load or store instruction

decodes the instruction, works out the transfer address, then either
loads from memory into Rt or stores Rt into memory, sized by sf

*/

exec_result_t execute_load_store(machine_state_t *state, decoded_instr_t instr) {

    ls_instr_fields_t fields = decode_load_store(instr);

    // resolve the address: load literal is PC + simm19 * 4, the rest use Xn
    addr_t address;
    if (fields.type == LS_LOAD_LITERAL) {
        dword_t offset = sign_extend(fields.simm19, 19) * 4;
        address = (addr_t) (read_pc(&state->special_registers) + offset);
    } else {
        address = compute_address(state, fields);
    }

    if (fields.L == 1) {

        // load: read 8 bytes (X) or 4 bytes (W) from memory into Rt
        dword_t value = fields.sf ? read_double_word(&state->memory, address)
                                  : (dword_t) read_word(&state->memory, address);
        write_reg_sf(&state->general_registers, fields.rt, fields.sf, value);
    } else {

        // store: write Rt into memory as 8 bytes (X) or 4 bytes (W)
        dword_t value = read_reg_sf(&state->general_registers, fields.rt, fields.sf);
        if (fields.sf) {
            write_double_word(&state->memory, address, value);
        } else {
            write_word(&state->memory, address, (word_t) value);
        }
    }

    // load/store never branches, so continue to the next instruction
    return EXEC_NEXT;
}