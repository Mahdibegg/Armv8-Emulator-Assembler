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
        fields.hw = extract_bits(instr.instr, 21, 22);
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
        .opr_LSB = extract_bits(instr.instr, 21, 21),
        .rm = extract_bits(instr.instr, 16, 20),
        .operand = extract_bits(instr.instr, 10,15),
        .rn = extract_bits(instr.instr, 5, 9),
        .rd = extract_bits(instr.instr, 0, 4)
    };

    // differentiate between arithmetic/logic and multiply
    if (fields.M == 0 && fields.opr_MSB == 1 && fields.opr_LSB == 0 ) {

        // fields.type updated to arithmetic
        fields.type = REG_ARITHMETIC;

        // arithmetic/logic overlap fields - opr_MSB already set
        fields.shift = extract_bits(instr.instr, 22, 23);
    } else if (fields.M == 0 && fields.opr_MSB == 0) {

        // field.type updated to logic (from arithmetic)
        fields.type = REG_LOGIC;
        
        // arithmetic/logic overlap fields - opr_MSB already set
        fields.shift = extract_bits(instr.instr, 22, 23);

        // setting N fields (for negation)
        fields.N = fields.opr_LSB;
    } else if (fields.M == 1 && fields.opr == MULTIPLY_OPR){

        // field.type updated to multiply
        fields.type = REG_MULTIPLY;

        // multiplication extracts x and ra bits (ignoring the the shift and N fields)
        fields.x = extract_bits(instr.instr, 15, 15);
        fields.ra = extract_bits(instr.instr, 10,14);
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
unsupported_shift_error -> shift function is not defined for emulator to execute, showing the shift field and address
invalid_field_error -> any fields other than opcode/shift that are invalid, show both field name, field value and instr 

*/

static void unsupported_opcode_error(byte_t opcode, word_t address) {

    // provide invalid opcode number and the instruction that failed to execute
    // so you are able to see which opcode is not available, and the address it failed at
    fprintf(stderr, "Invalid operation: unsupported opcode (0x%02x) at address 0x%08x\n",
        opcode,
        address
    );
    exit(EXIT_FAILURE);
}

static void unsupported_shift_error(byte_t shift, word_t address) {

    // provide invalid opcode number and the instruction that failed to execute
    // so you are able to see which opcode is not available, and the address it failed at
    fprintf(stderr, "Invalid operation: unsupported shift (0x%02x) at address 0x%08x\n",
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

2 helper functions for arithmetic 

one version is designed for 32, the other for 64 bit arithmetic
the parameters are overlapping fields values from the imm/reg decode structs
then produces a result and then in the outer case you can continue using the outputted result (such as writing to register etc.)
these functions will update the pstate register (common to both immediate/register arithmetic)

execute_general_arithmetic_64 is not commented since its just the 64 bit version of execute_general_arithmetic_32

*/

static word_t execute_general_arithmetic_32(machine_state_t *state, byte_t opcode, word_t rn, word_t rm) {
    
    // final value to return
    word_t result;

    // cases for opc, 00 - add, 01 - add and set flags, 10 - sub, 11 - sub and set flags
    switch (opcode) {
        case ADD_S: 
        case ADD:

            // ADD_S and ADD both create the result by adding
            result = rn + rm;    
            break;    
        case SUB_S:
        case SUB:

            // SUB_S and SUB both create result by subtracting
            result = rn - rm;
            break;
        default:

            unsupported_opcode_error(opcode, read_pc(&state->special_registers));
    }

    // updating processor state register only if opcode fits add_s and sub_s
    // otherwise ignore for the other instructions
    if (opcode == ADD_S || opcode == SUB_S) {

        // take sign bit of result in 32 bit
        bit_t n = sign_bit_32(result);

        bit_t z = result == 0;

        // addition, check overflow past 32 bits (carry)
        // subtraction, check borrow (rn >= rm)
        bit_t c;
        if (opcode == ADD_S) {

            // do the addition in 64 bits and check if it spills past bit 31
            c = ((dword_t) rn + (dword_t) rm) > UINT32_MAX;;
        } else {

            // borrow occurs when rn < rm (unsigned)
            c = (word_t) rn >= (word_t) rm;
        }

        // addition, check cases where signs are the same
        // subtraction, check cases where signs are different
        bit_t v;
        if (opcode == ADD_S) {
            v = ((sword_t) rn > 0 && (sword_t) rm > 0 && (sword_t) result < 0) ||
                ((sword_t) rn < 0 && (sword_t) rm < 0 && (sword_t) result > 0);
        } else { 
            v = ((sword_t) rn > 0 && (sword_t) rm < 0 && (sword_t) result < 0) ||
                ((sword_t) rn < 0 && (sword_t) rm > 0 && (sword_t) result > 0);
        }

        write_pstate(&state->special_registers, n, z, c, v);
    }

    return result;
}

static dword_t execute_general_arithmetic_64(machine_state_t *state, byte_t opcode, dword_t rn, dword_t rm) {

    dword_t result;

    switch (opcode) {
        case ADD_S: 
        case ADD:

            result = (dword_t) rn + (dword_t) rm;
            break;
        case SUB_S:
        case SUB:

            result = (dword_t) rn - (dword_t) rm;
            break;
        default:

            unsupported_opcode_error(opcode, read_pc(&state->special_registers));
    }
    
    if (opcode == ADD_S || opcode == SUB_S) {

        bit_t n = sign_bit_64(result);

        bit_t z = result == 0;

        bit_t c;
        if (opcode == ADD_S) {

            c = (UINT64_MAX - (dword_t) rn) < (dword_t) rm;
        } else {

            c = (dword_t) rn >= (dword_t) rm;
        }

        bit_t v;
        if (opcode == ADD_S) {

            v = ((sdword_t) rn > 0 && (sdword_t) rm > 0 && (sdword_t) result < 0) ||
                ((sdword_t) rn < 0 && (sdword_t) rm < 0 && (sdword_t) result > 0);
        } else { 

            v = ((sdword_t) rn > 0 && (sdword_t) rm < 0 && (sdword_t) result < 0) ||
                ((sdword_t) rn < 0 && (sdword_t) rm > 0 && (sdword_t) result > 0);
        }

        write_pstate(&state->special_registers, n, z, c, v);
    }

    return result;
}

/*

5 execute functions below for the different type, each one ideally has a switch case and 

uses the desired field to do real operations that would update the state

*/

static void execute_imm_arithmetic(machine_state_t *state, imm_instr_fields_t fields) {

    // sf = 0 -> 32 bit result to 32 bit register
    // sf = 1 -> 64 bit result to 64 bit register
    if (fields.sf == 0) {

        word_t rn = read_w_register(&state->general_registers, fields.rn);

        // Op2 is imm12, shifted left by 12 if sh is set
        word_t op2 = fields.sh ? (word_t) fields.imm12 << 12 : (word_t) fields.imm12;

        // execute arithmetic and update pstate if needed
        word_t result = execute_general_arithmetic_32(state, fields.opc, rn, op2);
    
        write_w_register(&state->general_registers, fields.rd, result);
        
    } else {

        // similar as in the other branch but 64 bit version

        dword_t rn = read_x_register(&state->general_registers, fields.rn);

        dword_t op2 = fields.sh ? (dword_t) fields.imm12 << 12 : (dword_t) fields.imm12;

        dword_t result = execute_general_arithmetic_64(state, fields.opc, rn, op2);
        
        write_x_register(&state->general_registers, fields.rd, result);
    }
}

static void execute_imm_wide_move(machine_state_t *state, imm_instr_fields_t fields) {

    // Op = imm16 shifted left by hw * 16
    byte_t shift = fields.hw * 16;

    // sf = 0 -> 32 bit result to 32 bit register
    // sf = 1 -> 64 bit result to 64 bit register
    if (fields.sf == 0) {

        // operand has to be shifted by shift variables defined outside branch casted to 32 bit
        word_t op = (word_t) fields.imm16 << shift;
        word_t rd = read_w_register(&state->general_registers, fields.rd);

        word_t result;

        switch (fields.opc) {
            case MOVN:

                // bitwise negate Op, upper 32 bits zeroed by word_t cast
                result = ~op;
                break;

            case MOVZ:

                // set Rd to Op
                result = op;
                break;

            case MOVK:

                // keep all bits of Rd except the 16 bits between shift and shift+15
                // clear those 16 bits then insert imm16 into that position
                result = (rd & ~((word_t) 0xFFFF << shift)) | op;
                break;

            default:

                unsupported_opcode_error(fields.opc, read_pc(&state->special_registers));
        }

        write_w_register(&state->general_registers, fields.rd, result);
        
    } else {
        
        // similar as in the other branch but 64 bit version

        dword_t op = (dword_t) fields.imm16 << shift;
        dword_t rd = read_x_register(&state->general_registers, fields.rd);

        dword_t result;

        switch (fields.opc) {
            case MOVN:

                result = ~op;
                break;

            case MOVZ:

                result = op;
                break;

            case MOVK:

                // 64 bit masking
                result = (rd & ~((dword_t) 0xFFFF << shift)) | op;
                break;

            default:

                unsupported_opcode_error(fields.opc, read_pc(&state->special_registers));
        }

        write_x_register(&state->general_registers, fields.rd, result);
    }
}

static void execute_reg_arithmetic(machine_state_t *state, reg_instr_fields_t fields) {

    // for 32-bit, shift amount is only lower 5 bits
    // for 64-bit, shift amount stays at 6 bits
    byte_t shift_amount = (fields.sf == 0) ? fields.operand & 0x1F : fields.operand & 0x3F;

    // sf = 0 -> 32 bit result to 32 bit register
    // sf = 1 -> 64 bit result to 64 bit register
    if (fields.sf == 0) {

        // reading from registers (32 bit) from the rn, rm fields of the instruction
        word_t rn = read_w_register(&state->general_registers, fields.rn);
        word_t rm = read_w_register(&state->general_registers, fields.rm);

        word_t shifted_rm;
        word_t result;

        // case for arithmetic shift, 00 - lsl, 01 - lsr, 10 - asr, 11 - ror
        switch (fields.shift) {
            case LSL:

                shifted_rm = rm << shift_amount;
                break;
            case LSR:

                shifted_rm = rm >> shift_amount;
                break;    
            case ASR:

                // making sure that the rm is casted to signed 32 bit size
                shifted_rm = (word_t) ((sword_t) rm >> shift_amount);
                break;    
            case ROR:

                // rotate only lower 32 bits, WORD_BITS is a constant for 32
                // truncate back to 32 bits by casing as word_t
                shifted_rm = (word_t) ((rm >> shift_amount) | (rm << (WORD_BITS - shift_amount)));
                result = shifted_rm;
                break;
            default: 

                unsupported_shift_error(fields.shift, read_pc(&state->special_registers));
        }
        
        if (fields.shift != ROR) {
            // obtain the result from checking the general arithmetic opcode case and producing the desired result
            // pstate registers are updated within this function execution
            result = execute_general_arithmetic_32(state, fields.opc, rn, shifted_rm);
        }

        // writing final result to the Rd register
        write_w_register(&state->general_registers, (unsigned) fields.rd, result);

    } else {

        // similar as in the other branch but 64 bit version

        dword_t rn = read_x_register(&state->general_registers, fields.rn);
        dword_t rm = read_x_register(&state->general_registers, fields.rm);

        dword_t shifted_rm;
        dword_t result;

        switch (fields.shift) {
            case LSL:

                shifted_rm = rm << shift_amount;
                break;
            case LSR:

                shifted_rm = rm >> shift_amount;
                break;
            case ASR:

                shifted_rm = (dword_t) ((sdword_t) rm >> shift_amount);
                break;
            case ROR:

                shifted_rm = (dword_t) ((rm >> shift_amount) | (rm << (DWORD_BITS - shift_amount)));
                result = shifted_rm;
                break;    
            default: 

                unsupported_shift_error(fields.shift, read_pc(&state->special_registers));
        }

        if (fields.shift != ROR) {

            result = execute_general_arithmetic_64(state, fields.opc, rn, shifted_rm);
        }

        write_x_register(&state->general_registers, (unsigned) fields.rd, result);
    }
}

static void execute_reg_logic(machine_state_t *state, reg_instr_fields_t fields, instr_t instr) {

    // combine shift opcode and N bits to create 3 bit binary digit for case checks
    byte_t shift_opcode = (fields.opc << 1) + fields.N;

    // case for logical shift
    // 000 - and, 001 - bic, 010 - orr, 011 - orn, 100 - eor, 101 - eon, 110 - ands, 111 - bics
    // sf = 0 -> 32 bit result to 32 bit register
    // sf = 1 -> 64 bit result to 64 bit register
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
                break;
            case BICS:
            case BIC:

                result = rn & ~op;
                break;
            case ORR:

                result = rn | op;
                break;
            case ORN:

                result = rn | ~op;
                break;
            case EOR:

                result = rn ^ op;
                break;
            case EON:

                result = rn ^ ~op;
                break;
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
        
        // similar as in the other branch but 64 bit version

        dword_t rn = read_x_register(&state->general_registers, fields.rn);
        dword_t op = read_x_register(&state->general_registers, fields.rm);

        dword_t result;

        switch (shift_opcode) {
            case ANDS:
            case AND:

                result = rn & op;
                break;
            case BICS:
            case BIC:

                result = rn & ~op;
                break;
            case ORR:

                result = rn | op;
                break;
            case ORN:

                result = rn | ~op;
                break;
            case EOR:

                result = rn ^ op;
                break;
            case EON:

                result = rn ^ ~op;
                break;
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
        if (fields.x == MADD)

            result = ra + (rn * rm);
        else if (fields.x == MSUB) {

            result = ra - (rn * rm);
        }

        // writing to rd using write_w (32 bit)
        write_w_register(&state->general_registers, (unsigned) fields.rd, result);
    } else {

        // similar as in the other branch but 64 bit version
        
        dword_t ra = read_x_register(&state->general_registers, (unsigned) fields.ra);
        dword_t rn = read_x_register(&state->general_registers, (unsigned) fields.rn);
        dword_t rm = read_x_register(&state->general_registers, (unsigned) fields.rm);

        dword_t result;

        if (fields.x == MADD)

            result = ra + (rn * rm);
        else if (fields.x == MSUB) {

            result = ra - (rn * rm);
        }

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

            execute_imm_arithmetic(state, fields);
            break;
        // specifically execute the immediate with hw, imm16 for (movn, movz, movk)
        case IMM_WIDE_MOVE:

            execute_imm_wide_move(state, fields);
            break;
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

            execute_reg_arithmetic(state, fields);
            break;
        // using logical shift and N field for executing
        // (and, bic, orr, orn, eor, eon, ands, bics)
        case REG_LOGIC:

            execute_reg_logic(state, fields, instr);
            break;
        // using the x field for executing (madd, msub)
        // then using the ra field as a third input register for multiply instructions
        case REG_MULTIPLY:

            execute_reg_multiply(state, fields, instr);
            break;
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
            instr.instr
        );
        exit(EXIT_FAILURE);
    }

    // no branching, continue to next instruction in pipeline
    // PC should be updated outside loop
    return EXEC_NEXT;
}