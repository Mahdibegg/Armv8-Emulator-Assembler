#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#include "state.h"
#include "pipeline/decode_struct.h"
#include "types.h"

// opi instruction field to differentiate operations
#define ARITHMETIC_OPI 0b010
#define WIDE_MOVE_OPI 0b101

// Immediate DP instruction fields as struct 
typedef struct {

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
} imm_instr_fields;

// Register DP instruction fields as struct
typedef struct {

    // General register instruction format
    bit_t sf;
    byte_t opc;
    byte_t opr;
    byte_t rm;
    byte_t rn;
    byte_t rd;

    // Arithmetic or logic shift (true/false)
    bit_t is_arithmetic;
    byte_t shift;
    
    // Logical shift
    bit_t N; 

    // Multiply
    bit_t x;
    byte_t ra;
} reg_instr_fields;

// extracts bits to create immediate instruction fields as struct
imm_instr_fields decode_imm_instr(decoded_instr_t instr);

// extract bits to create register instruction fields as struct
reg_instr_fields decode_reg_instr(decoded_instr_t instr);

// takes bits and executes correct data processing type instruction
exec_result_t execute_data_processing(machine_state_t *state, decoded_instr_t instr);

#endif