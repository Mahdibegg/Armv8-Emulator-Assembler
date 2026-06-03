#ifndef REGISTERS_H
#define REGISTERS_H

#include "types.h"
#include <stdbool.h>
#define REG_NUM 31
#define ZERO_REGISTER 31

// general purpose registers 0-30
typedef struct {
    reg64_t r[REG_NUM];
} gen_regs;

// processor state register (4 fields)
typedef struct {
    bit_t n_flag;
    bit_t z_flag;
    bit_t c_flag;
    bit_t v_flag;
} pstate;

// special registers: zero, program counter, and stack pointer, processor state
typedef struct {
    reg64_t pc;
    pstate psr;
} spec_reg;

// REQUIRED register functions to be implemented in registers.c

// initialisation to 0 for both general and special registers
void init_gen_registers(gen_regs *registers);
void init_spec_registers(spec_reg *registers);

// General purpose register functions

// read/write on 64 bit registers (referred to as X in the spec)
reg64_t read_x_register(const gen_regs *registers, unsigned index);
void write_x_register(gen_regs *registers, unsigned index, dword_t value);

// read/write on 32 bit registers (referred to as W in the spec)
reg32_t read_w_register(const gen_regs *registers, unsigned index);
void write_w_register(gen_regs *registers, unsigned index, word_t value);

// wrapper for read/write, chooses between 64 and 32 bit
dword_t read_reg_sf(gen_regs *registers, unsigned index, bit_t sf);
void write_reg_sf(gen_regs *registers, unsigned index, bit_t sf, dword_t value);

// Special purpose register functions

// read/write on 64 bit PC register
reg64_t read_pc(const spec_reg *registers);
void write_pc(spec_reg *registers, dword_t value);

// writing PSTATE register, taking all flags specifically and setting them
void write_pstate(spec_reg *registers, bit_t n, bit_t z, bit_t c, bit_t v);

#endif