#include <stdio.h>
#include <stdlib.h>

#include "data_processing.h"
#include "bit_utils/bit.h"
#include "../../registers/registers.h"

// immediate instruction field cases (OPI)
#define ARITHMETIC_OPI 0x2
#define WIDE_MOVE_OPI 0x5

// register instruction field cases (OPR)
#define MULTIPLY_OPR 0x8

// immediate arithmetic instructions (opcode field)
#define ADD 0x0
#define ADD_S 0x1
#define SUB 0x2
#define SUB_S 0x3

// immediate wide move instructions (opcode field)
#define MOVN 0x0
#define MOVZ 0x2
#define MOVK 0x3

// register arithmetic shift instructions (shift field)
#define LSL 0x0
#define LSR 0x1
#define ASR 0x2
#define ROR 0x3

// register logical shift instructions (shift field)
#define AND 0x0
#define BIC 0x1
#define ORR 0x2
#define ORN 0x3
#define EOR 0x4
#define EON 0x5
#define ANDS 0x6
#define BICS 0x7

// register multiplication instruction (x field)
#define MADD 0x0
#define MSUB 0x1

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

Error messages for execution phase

unsupported_opcode_error -> opcode not defined for emulator to execute, show both invalid opcode and address

*/

static void unsupported_opcode_error(byte_t opcode, word_t address) {

        // provide invalid opcode number and the instruction that failed to execute
        // so you are able to see which opcode is not available, and the address it failed at
        fprintf(stderr, "Invalid operation: unsupported opcode (0x%02x) at address 0x%016lx\n",
            opcode,
            address
        );
        exit(EXIT_FAILURE);
}

static void unsupported_shift_error(byte_t shift, word_t address) {

        // provide invalid opcode number and the instruction that failed to execute
        // so you are able to see which opcode is not available, and the address it failed at
        fprintf(stderr, "Invalid operation: unsupported shift (0x%02x) at address 0x%016lx\n",
            shift,
            address
        );
        exit(EXIT_FAILURE);
}

static void invalid_field_error(const char *field_name, word_t field_value, instr_t instr) {
        
    // provide invalid opcode number and the instruction that failed to execute
        // so you are able to see which opcode is not available, and the address it failed at
        fprintf(stderr, "Invalid field: unsupported operation due to %s field with value (0x%08x) in instruction (0x%08x)",
            field_name,
            field_value,
            instr
        );
        exit(EXIT_FAILURE);
}

/*

5 execute functions below for the different type, each one ideally has a switch case and 

uses the desired field to do real operations that would update the state

*/

static void execute_imm_arithmetic(machine_state_t *state, imm_instr_fields_t fields, instr_t instr) {

    // cases for opc, 00 - add, 01 - add and set flags, 10 - sub, 11 - sub and set flags
    switch (fields.opc) {
        case ADD:
        case ADD_S:
        case SUB:
        case SUB_S:
        default:

            unsupported_opcode_error(fields.opc, read_pc(&state->special_registers));
    }
}

static void execute_imm_wide_move(machine_state_t *state, imm_instr_fields_t fields, instr_t instr) {

    // cases for opc, 00 - movn (move with not), 10 - movz (move with zero), 11 - movk (move with keep)
    switch (fields.opc) {
        case MOVN:
        case MOVZ:
        case MOVK:
        default:
            
            unsupported_opcode_error(fields.opc, read_pc(&state->special_registers));
    }
}

static void execute_reg_arithmetic(machine_state_t *state, reg_instr_fields_t fields, instr_t instr) {

    if (fields.sf == 0) {

        // reading from registers (32 bit) from the rn, rm fields of the instruction
        word_t rn = read_w_register(&state->general_registers, fields.rn);
        word_t rm = read_w_register(&state->general_registers, fields.rm);

        // amount to shift by is the operand field of the instruction
        byte_t shift_amount = fields.opr;
        word_t shifted_rm;

        // case for arithmetic shift, 00 - lsl, 01 - lsr, 10 - asr, 11 - ror
        switch (fields.shift) {
            case LSL:
                shifted_rm = rm << shift_amount;
            case LSR:
                shifted_rm = rm >> shift_amount;
            case ASR:
                // making sure that the rm is casted to signed 32 bit size
                shifted_rm = (word_t) ((int32_t) rm >> shift_amount);
            case ROR:
                // rotate only lower 32 bits, sizeof(word_t)*8 give bytes * 8, so bit size of word_t
                shifted_rm = (rm >> shift_amount) | (rm << (sizeof(word_t)*8 - shift_amount));
                // truncate back to 32 bits by casing as word_t
                shifted_rm = (word_t) shifted_rm;
            default: 

                unsupported_shift_error(fields.shift, read_pc(&state->special_registers));
        }

        // final value to return
        word_t result;

        // then adding the shifted result to the Rm to complete the instruction
        // cases for opc, 00 - add, 01 - add and set flags, 10 - sub, 11 - sub and set flags
        switch (fields.opc) {
            case ADD_S: 
            case ADD:
                result = (word_t) rn + (word_t) shifted_rm;    
            case SUB_S:
            case SUB:
                result = (word_t) rn - (word_t) shifted_rm;
            default:

                unsupported_opcode_error(fields.opc, read_pc(&state->special_registers));
        }

        // writing final result to the Rd register
        write_w_register(&state->general_registers, (unsigned) fields.rd, (word_t) result);
    
        // updating processor state register only if opcode fits add_s and sub_s
        // otherwise ignore for the other instructions
        if (fields.opc == ADD_S || fields.opc == SUB_S) {

            // take sign bit of result in 32 bit
            bit_t n = sign_bit_32(result);

            bit_t z = result == 0;

            // addition, check overflow past 32 bits (carry)
            // subtraction, check borrow (rn >= rm)
            bit_t c;
            if (fields.opc == ADD_S) {

                // do the addition in 64 bits and check if it spills past bit 31
                c = ((dword_t) (word_t) rn + (word_t) shifted_rm) > 0xFFFFFFFF;
            } else {

                // borrow occurs when rn < rm (unsigned)
                c = (word_t) rn >= (word_t) shifted_rm;
            }

            // addition, check cases where signs are the same
            // subtraction, check cases where signs are different
            bit_t v;
            if (fields.opc == ADD_S) {
                v = ((sword_t) rn > 0 && (sword_t) shifted_rm > 0 && (sword_t) result < 0) ||
                    ((sword_t) rn < 0 && (sword_t) shifted_rm < 0 && (sword_t) result > 0);
            } else { 
                v = ((sword_t) rn > 0 && (sword_t) shifted_rm < 0 && (sword_t) result < 0) ||
                    ((sword_t) rn < 0 && (sword_t) shifted_rm > 0 && (sword_t) result > 0);
            }

            write_pstate(&state->special_registers, n, z, c, v);
        }

    } else {

        // similar as above but 64 bit version

        dword_t rn = read_x_register(&state->general_registers, fields.rn);
        dword_t rm = read_x_register(&state->general_registers, fields.rm);

        switch (fields.shift) {
            case LSL:
            case LSR:
            case ASR:
            case ROR:
            default: 

                unsupported_shift_error(fields.shift, read_pc(&state->special_registers));
        }
    }
}

static void execute_reg_logic(machine_state_t *state, reg_instr_fields_t fields, instr_t instr) {

    // combine shift opcode and N bits to create 3 bit binary digit for case checks
    byte_t shift_opcode = (fields.opc << 1) + fields.N;

    // case for logical shift
    // 000 - and, 001 - bic, 010 - orr, 011 - orn, 100 - eor, 101 - eon, 110 - ands, 111 - bics

    if (fields.sf == 0) {

        // read value of register Rn to store into Rn and operand value from Rm register into op 
        word_t rn = read_w_register(&state->general_registers, fields.rn);
        word_t op = read_w_register(&state->general_registers, fields.rm);

        word_t result;

        // go through each case after extracting the values from the correct registers
        // set result to the operation that it is required to be
        switch (shift_opcode) {
            case ANDS:
            case AND:
                result = rn & op;
            case BICS:
            case BIC:
                result = rn & ~op;
            case ORR:
                result = rn | op;
            case ORN:
                result = rn | ~op;
            case EOR:
                result = rn ^ op;
            case EON:
                result = rn ^ ~op;
            default:
                invalid_field_error("Opcode", shift_opcode, instr);
        }

        // set rd = rn & operand (named op)
        write_w_register(&state->general_registers, (unsigned) fields.rd, result);

        // for ANDS and BICS, the pstate register will need to be updated
        // in the case switching, they do the exact same thing as AND and BIC respectively
        if (shift_opcode == ANDS || shift_opcode == BICS) {

            // set flags, n = field.n, c = v = 0, z = 1 if result = 0
            bit_t n = sign_bit_32(result);
            bit_t z = result == 0;
            write_pstate(&state->special_registers, n, z, 0, 0);
        }
    } else {
        
        // similar as above but 64 bit version

        dword_t rn = read_x_register(&state->general_registers, fields.rn);
        dword_t op = read_x_register(&state->general_registers, fields.rm);

        dword_t result;

        switch (shift_opcode) {
            case ANDS:
            case AND:
                result = rn & op;
            case BICS:
            case BIC:
                result = rn & ~op;
            case ORR:
                result = rn | op;
            case ORN:
                result = rn | ~op;
            case EOR:
                result = rn ^ op;
            case EON:
                result = rn ^ ~op;
            default:
                invalid_field_error("Opcode", shift_opcode, instr);
        }
    
        write_x_register(&state->general_registers, (unsigned) fields.rd, result);
        if (shift_opcode == ANDS || shift_opcode == BICS) {

            bit_t n = sign_bit_64(result);
            bit_t z = result == 0;
            write_pstate(&state->special_registers, n, z, 0, 0);
        }
    }
}

static void execute_reg_multiply(machine_state_t *state, reg_instr_fields_t fields, instr_t instr) {
      
    // sf = 0 -> 32 bit result to 32 bit register
    // sf = 1 -> 64 bit result to 64 bit register
    if (fields.sf == 0) {
        
        // separating register reads for clarity (32 bit)
        word_t ra = read_w_register(&state->general_registers, (unsigned) fields.ra);
        word_t rn = read_w_register(&state->general_registers, (unsigned) fields.rn);
        word_t rm = read_w_register(&state->general_registers, (unsigned) fields.rm);

        word_t result;

        // result is of the form ra + (rn * rm) for MADD
        // result is of the form ra - (rn * rm) for MSUB
        switch (fields.x) {

            case MADD:
                word_t result = ra + (rn * rm);
            case MSUB:
                word_t result = ra - (rn * rm);
            default: 
                invalid_field_error("x", fields.x, instr);
        }

        // writing to rd using write_w (32 bit)
        write_w_register(&state->general_registers, (unsigned) fields.rd, result);
    } else {
        
        // separating register reads for clarity (64 bit)
        dword_t ra = read_x_register(&state->general_registers, (unsigned) fields.ra);
        dword_t rn = read_x_register(&state->general_registers, (unsigned) fields.rn);
        dword_t rm = read_x_register(&state->general_registers, (unsigned) fields.rm);

        dword_t result;

        switch (fields.x) {

            case MADD:
                word_t result = ra + (rn * rm);
            case MSUB:
                word_t result = ra - (rn * rm);
            default: 
                invalid_field_error("x", fields.x, instr);
        }

        // writing to rd using write_x (64 bit)
        write_x_register(&state->general_registers, (unsigned) fields.rd, result);
    }
}

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
            fprintf(stderr, "Invalid operation: unsupported immediate instruction (0x%08x) executed at address 0x%016lx\n", 
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
            fprintf(stderr, "Invalid operation: unsupported register instruction (0x%08x) at address 0x%016lx\n",
                instr,
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
        fprintf(stderr, "Invalid data processing instruction: unsupported instruction type (non-immedate and non-register) 0x%08x", 
            instr
        );
        exit(EXIT_FAILURE);
    }

    // no branching, continue to next instruction in pipeline
    // PC should be updated outside loop
    return EXEC_NEXT;
}