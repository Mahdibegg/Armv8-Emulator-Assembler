#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#include "state.h"
#include "pipeline/decode_struct.h"
#include "types.h"

// instruction field constants to differentiate operations during decode/execute

// immediate instruction field cases (OPI)
#define ARITHMETIC_OPI 0x2
#define WIDE_MOVE_OPI 0x5

// register instruction field cases (OPR)
#define MULTIPLY_OPR 0x8

// Immediate instruction sub types (arithmetic/wide move)
typedef enum {
    IMM_ARITHMETIC,
    IMM_WIDE_MOVE
} immediate_type_t;

// Immediate DP instruction fields as struct 
// Should be returned by an immediate_instruction decoder
typedef struct {

    // Immediate instruction type (arithmetic/wide move)
    immediate_type_t type;

    // General immediate instruction format
    bit_t sf;
    byte_t opc;
    byte_t opi;
    byte_t rd;

    // Arithmetic operand format
    bit_t sh;
    word_t imm12;
    byte_t rn;

    // Wide Move operand format
    byte_t hw;
    word_t imm16;
} imm_instr_fields_t;

// Register instruction sub types (arithmetic/logic/multiply)
typedef enum {
    REG_ARITHMETIC,
    REG_LOGIC,
    REG_MULTIPLY
} register_type_t;

// Register DP instruction fields as struct
// Should be returned by a register_instruction decoder
typedef struct {

    // Register instruction type (arithmetic/logic/multiply)
    register_type_t type;

    // General register instruction format
    bit_t sf;
    byte_t opc;
    bit_t M;
    byte_t opr;
    byte_t rm;
    byte_t rn;
    byte_t rd;

    // Arithmetic or logic shift (true/false)
    bit_t opr_MSB;
    byte_t shift;
    
    // Logical shift
    bit_t N; 

    // Multiply
    bit_t x;
    byte_t ra;
} reg_instr_fields_t;

// takes bits and executes correct data processing type instruction
exec_result_t execute_data_processing(machine_state_t *state, decoded_instr_t instr);

#endif