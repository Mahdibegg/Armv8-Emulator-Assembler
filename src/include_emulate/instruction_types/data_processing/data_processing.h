#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#include <stdbool.h>
#include <stdint.h>

#include "state.h"
#include "pipeline/decode_struct.h"

// Immediate DP instruction fields as struct 
typedef struct {

    // General immediate instruction format
    bool sf;
    uint8_t opc;
    uint8_t opi;
    uint8_t rd;

    // Arithmetic operand format
    bool sh;
    uint32_t imm12;
    uint8_t rn;

    // Wide Move operand format
    uint8_t hw;
    uint32_t imm16;
} imm_instr_fields;

// Register DP instruction fields as struct
typedef struct {

    // General register instruction format
    bool sf;
    uint8_t opc;
    uint8_t opr;
    uint8_t rm;
    uint8_t rn;
    uint8_t rd;

    // Arithmetic or logic shift (true/false)
    bool is_arithmetic;
    uint8_t shift;
    
    // Logical shift
    bool N; 

    // Multiply
    bool x;
    uint8_t ra;
} reg_instr_fields;

exec_result_t execute_data_processing(machine_state_t *state, decoded_instr_t instr);

#endif