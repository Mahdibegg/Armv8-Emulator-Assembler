#ifndef REGISTERS_H
#define REGISTERS_H

#include "shared/types.h"
#include <stdbool.h>

/* general purpose registers 0-30 */
typedef struct {
    reg64_t r[REG_NUM];
} gen_regs;

/* processor state register (4 fields) */
typedef struct {
    bit_t n_flag;
    bit_t z_flag;
    bit_t c_flag;
    bit_t v_flag;
} pstate;

/* special registers: zero, program counter, and stack pointer, processor state */
typedef struct {
    reg64_t pc;
    pstate psr;
} spec_reg;

/* REQUIRED register functions to be implemented in registers.c */

/*
 * initialisation to 0 for both general and special registers
 * init_gen_registers zeroes every general purpose register
 * init_spec_registers zeroes the special registers, then sets the Z flag
 */
void init_gen_registers(gen_regs *registers);
void init_spec_registers(spec_reg *registers);

/* General purpose register functions */

/*
 * read/write on 64 bit registers (referred to as X in the spec)
 * read_x_register returns the full 64 bit value at index (0 for the zero register)
 * write_x_register stores value at index (writes to the zero register are ignored)
 */
reg64_t read_x_register(const gen_regs *registers, unsigned index);
void write_x_register(gen_regs *registers, unsigned index, dword_t value);

/*
 * read/write on 32 bit registers (referred to as W in the spec)
 * read_w_register returns the lower 32 bits at index (0 for the zero register)
 * write_w_register stores value at index, zero-extending into the upper 32 bits
 */
reg32_t read_w_register(const gen_regs *registers, unsigned index);
void write_w_register(gen_regs *registers, unsigned index, word_t value);

/*
 * wrapper for read/write, chooses between 64 and 32 bit
 * read_reg_sf reads as X when sf is set, otherwise as W
 * write_reg_sf writes as X when sf is set, otherwise as W
 */
dword_t read_reg_sf(gen_regs *registers, unsigned index, bit_t sf);
void write_reg_sf(gen_regs *registers, unsigned index, bit_t sf, dword_t value);

/* Special purpose register functions */

/*
 * read/write on 64 bit PC register
 * read_pc returns the current program counter
 * write_pc sets the program counter to value
 */
reg64_t read_pc(const spec_reg *registers);
void write_pc(spec_reg *registers, dword_t value);

/* writing PSTATE register, taking all flags specifically and setting them */
void write_pstate(spec_reg *registers, bit_t n, bit_t z, bit_t c, bit_t v);

#endif
