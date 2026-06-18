#ifndef SHIFT_OPCODES_H
#define SHIFT_OPCODES_H

#define HALT_INSTR 0x8a000000

/* 
 * Register arithmetic shift instructions (shift field)
 * Shared between encoder and data_processing
 */
#define LSL 0x0
#define LSR 0x1
#define ASR 0x2
#define ROR 0x3

#endif