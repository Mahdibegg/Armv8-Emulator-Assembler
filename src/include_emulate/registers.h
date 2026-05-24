#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>
#include <stdbool.h>
#define REG_NUM 31

// registers as 64 bit ints and 32 bit ints
typedef uint64_t reg64_t;
typedef uint32_t reg32_t;

// general purpose registers 0-31, 64 bits
typedef struct {
    reg64_t r[REG_NUM];
} gen_reg;

// special registers: zero, program counter, and stack pointers
typedef struct {
    reg64_t zr = 0;
    reg64_t pc;
    reg64_t sp;
    reg32_t wsp;
    pstate psr;
} spec_reg;

// processor state register
typedef struct {
    bool n_flag;
    bool z_flag;
    bool c_flag;
    bool v_flag;
} pstate;

#endif